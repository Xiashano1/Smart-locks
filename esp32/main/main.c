#include "lcd.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "gui.h"
#include "password.h"
#include "touch.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "mqtt_app.h"

static const char *TAG = "MAIN";

//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//测试硬件：ESP32
//QDtech-TFT液晶驱动 for ESP32
//xiao冯@ShenZhen QDtech co.,LTD
//引脚接线请参考 esp32_config.h

/* WiFi 配置 — 改成你的 WiFi 名称和密码 */
#define WIFI_SSID      "夏杉"
#define WIFI_PASS      "66666666"

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WiFi disconnected, retrying...");
        // LCD 显示断连
        POINT_COLOR = RED;
        LCD_ShowString(10, 50, 16, (u8 *)"WiFi: Disconnected!", 1);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    // LCD 显示 WiFi 连接中
    POINT_COLOR = BLUE;
    LCD_ShowString(10, 50, 16, (u8 *)"WiFi: Connecting...", 1);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to WiFi...");

    /* 等待连接成功（最多 30 秒） */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT,
                                           pdFALSE, pdFALSE, pdMS_TO_TICKS(30000));
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "WiFi connected successfully");
        POINT_COLOR = GREEN;
        LCD_ShowString(10, 50, 16, (u8 *)"WiFi: Connected!  ", 1);
    } else {
        ESP_LOGW(TAG, "WiFi connection timeout, continuing anyway...");
        POINT_COLOR = RED;
        LCD_ShowString(10, 50, 16, (u8 *)"WiFi: Timeout!    ", 1);
    }
}

void app_main(void)
{
	ESP_LOGI(TAG, "[APP] Startup..");
	ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

	// 设置日志等级
	esp_log_level_set("*", ESP_LOG_INFO);
	esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
	esp_log_level_set("mqtt_example", ESP_LOG_VERBOSE);
	esp_log_level_set("transport_base", ESP_LOG_VERBOSE);
	esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
	esp_log_level_set("transport", ESP_LOG_VERBOSE);
	esp_log_level_set("outbox", ESP_LOG_VERBOSE);

	// 初始化 NVS（用于触摸校准数据存储 + WiFi）
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_LOGI(TAG, "System initialized");

	LCD_Init();	   //液晶屏初始化

	// LCD 显示启动状态
	POINT_COLOR = BLUE;
	LCD_ShowString(10, 10, 16, (u8 *)"ESP32 Starting...", 1);
	LCD_ShowString(10, 30, 16, (u8 *)"SSID: " WIFI_SSID, 1);

	/* 连接 WiFi（会更新 LCD 显示连接状态） */
	wifi_init_sta();

	/* 启动 MQTT 连接 OneNet */
	mqtt_app_start();

	// 给 MQTT 一点时间初始化
	vTaskDelay(pdMS_TO_TICKS(500));

	while(1)
	{
		Password_Lock();
	}
 }
