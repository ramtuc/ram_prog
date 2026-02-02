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
#include "esp_timer.h" // 時間計測用

// Nanopb関連
#include "pb_encode.h"
#include "sensor.pb.h"

// 設定ファイル（config.h.example をコピーして config.h を作成してください）
#include "config.h"

static const char *TAG = "GRPC_VS_JSON";

/* --- 証明書の埋め込み --- */
// CMakeLists.txt の EMBED_TXTFILES で指定したファイル名に基づきます
extern const uint8_t azure_root_ca_pem_start[] asm("_binary_azure_root_ca_pem_start");
extern const uint8_t azure_root_ca_pem_end[]   asm("_binary_azure_root_ca_pem_end");

/* --- Wi-Fi 接続管理用 --- */
static EventGroupHandle_t s_wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

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

/* --- 送信共通処理 --- */
esp_err_t perform_post(const char *url_suffix, const char *content_type, const char *data, int len) {
    char full_url[512]; // Keyが長いため少し大きめに確保
    snprintf(full_url, sizeof(full_url), "%s/%s?code=%s", AZURE_DOMAIN, url_suffix, FUNCTION_KEY);

    esp_http_client_config_t config = {
        .url = full_url,
        .method = HTTP_METHOD_POST,
        .cert_pem = (const char *)azure_root_ca_pem_start,
        .transport_type = HTTP_TRANSPORT_OVER_SSL, // AzureはHTTPS必須
        .skip_cert_common_name_check = false,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", content_type);
    esp_http_client_set_post_field(client, data, len);

    int64_t start = esp_timer_get_time();
    esp_err_t err = esp_http_client_perform(client);
    int64_t end = esp_timer_get_time();

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS POST OK. Network Time: %lld us, Status: %d, Size: %d bytes", 
                 (end - start), esp_http_client_get_status_code(client), len);
    } else {
        ESP_LOGE(TAG, "HTTPS POST Failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    return err;
}

void app_main(void) {
    // 1. NVSの初期化（Wi-Fi設定等の保存に必要）
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Wi-Fi接続
    wifi_init_sta();

    while(1) {
        float temp = 25.5f;
        int hum = 60;

        // --- A. JSON 比較用 ---
        char json_data[128];
        int64_t j_start = esp_timer_get_time();
        int j_len = snprintf(json_data, sizeof(json_data), 
                             "{\"device_id\":\"ESP32-S3-LAB\",\"temperature\":%.2f,\"humidity\":%d}", 
                             temp, hum);
        int64_t j_end = esp_timer_get_time();
        ESP_LOGI(TAG, "[JSON] Serialize Time: %lld us", (j_end - j_start));
        perform_post("esp_data_json", "application/json", json_data, j_len);

        vTaskDelay(pdMS_TO_TICKS(5000));

        // --- B. gRPC (Protobuf) 比較用 ---
        uint8_t pb_buffer[128];
        SensorData msg = SensorData_init_default; // 必須：デフォルト値で初期化
        snprintf(msg.device_id, sizeof(msg.device_id), "ESP32-S3-LAB");
        msg.temperature = temp;
        msg.humidity = hum;

        int64_t p_start = esp_timer_get_time();
        pb_ostream_t stream = pb_ostream_from_buffer(pb_buffer, sizeof(pb_buffer));
        if (!pb_encode(&stream, SensorData_fields, &msg)) {
            ESP_LOGE(TAG, "Nanopb Encoding failed!");
        } else {
            int64_t p_end = esp_timer_get_time();
            ESP_LOGI(TAG, "[gRPC] Serialize Time: %lld us", (p_end - p_start));
            perform_post("esp_data_grpc", "application/x-protobuf", (char*)pb_buffer, stream.bytes_written);
        }

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}