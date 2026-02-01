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
#include "cJSON.h"

// --- 設定項目 ---
#define WIFI_SSID      "your-wifi-ssid"
#define WIFI_PASS      "your-wifi-password"
#define WEB_URL        "http://192.168.1.100:8000/api/data"

static const char *TAG = "HTTP_JSON_BASE";

// Wi-Fi接続完了を待つためのイベントグループ
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

// Wi-Fiイベントハンドラ
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retry connecting to the AP...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Wi-Fi初期化関数（実体）
void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Waiting for Wi-Fi connection...");
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI(TAG, "Connected to SSID:%s", WIFI_SSID);
}

// HTTP POSTタスク（実体）
static void http_post_json_task(void *pvParameters) {
    while (1) {
        cJSON *root = cJSON_CreateObject();
        cJSON_AddNumberToObject(root, "temperature", 26.54);
        cJSON_AddNumberToObject(root, "humidity", 60);
        cJSON_AddStringToObject(root, "device_id", "ESP32-S3-001");

        char *post_data = cJSON_PrintUnformatted(root);
        ESP_LOGI(TAG, "Sending JSON: %s", post_data);

        esp_http_client_config_t config = {
            .url = WEB_URL,
            .method = HTTP_METHOD_POST,
        };
        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "HTTP POST Status = %d", esp_http_client_get_status_code(client));
        } else {
            ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(err));
        }

        esp_http_client_cleanup(client);
        cJSON_Delete(root);
        free(post_data);

        vTaskDelay(pdMS_TO_TICKS(10000)); // 10秒おき
    }
}

// エントリポイント
void app_main(void) {
    // 1. NVS初期化
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Wi-Fi接続（完了するまでここでブロック）
    wifi_init_sta();

    // 3. HTTP送信タスク開始
    xTaskCreate(&http_post_json_task, "http_post_json_task", 8192, NULL, 5, NULL);
}