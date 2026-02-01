#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"

// Nanopb関連
#include "pb_encode.h"
#include "sensor.pb.h"
#include "config.h"

static const char *TAG = "GRPC_LAB";

// WiFi設定とサーバーURLは config.h に記載してください
// config.h.example をコピーして config.h を作成し、設定値を入力してください

/* Wi-Fi接続完了イベント用 */
static EventGroupHandle_t s_wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

// Wi-Fiイベントハンドラ
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "retry to connect to the AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Wi-Fi初期化
void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}

// gRPC(Protobuf)データをPOST送信
void send_grpc_data(uint8_t *buffer, int len) {
    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .method = HTTP_METHOD_POST,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // ヘッダーをバイナリ（Protobuf）用に設定
    esp_http_client_set_header(client, "Content-Type", "application/x-protobuf");
    esp_http_client_set_post_field(client, (const char *)buffer, len);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

void app_main(void) {
    // 1. NVSの初期化（Wi-Fi設定の保存に必要）
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Wi-Fi接続
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();

    // 3. 30秒ごとにデータを送信するループ
    while (1) {
        uint8_t buffer[128];
        SensorData message = SensorData_init_default;
        
        // ランダムな値を生成
        float temperature = 20.0f + (esp_random() % 1500) / 100.0f;  // 20.0～34.99℃
        int32_t humidity = 30 + (esp_random() % 51);  // 30～80%
        
        // データのセット
        snprintf(message.device_id, sizeof(message.device_id), "ESP32-S3-GRPC-LAB");
        message.temperature = temperature;
        message.humidity = humidity;

        // 送信する数値を表示
        ESP_LOGI(TAG, "=== Sensor Data ===");
        ESP_LOGI(TAG, "Device ID: %s", message.device_id);
        ESP_LOGI(TAG, "Temperature: %.2f °C", message.temperature);
        ESP_LOGI(TAG, "Humidity: %d %%", message.humidity);

        // シリアライズ実行
        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        if (!pb_encode(&stream, SensorData_fields, &message)) {
            ESP_LOGE(TAG, "Encoding failed!");
            vTaskDelay(pdMS_TO_TICKS(30000));
            continue;
        }

        // バイナリデータを16進数で表示
        ESP_LOGI(TAG, "Binary data (%d bytes):", stream.bytes_written);
        for (int i = 0; i < stream.bytes_written; i++) {
            printf("%02X ", buffer[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");

        // 送信
        ESP_LOGI(TAG, "Sending gRPC data...");
        send_grpc_data(buffer, stream.bytes_written);
        ESP_LOGI(TAG, "Data sent. Waiting 30 seconds...");

        // 30秒待機
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}