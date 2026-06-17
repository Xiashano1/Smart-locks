/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "cJSON.h"

static const char *TAG = "mqtt_example";

static esp_mqtt_client_handle_t mqtt_client = NULL;

/** MQTT 连接后订阅/初始化是否已完成（防止重复订阅） */
static bool mqtt_session_inited = false;

/** OneNet 要求 id 为纯数字，每次发布递增 */
static uint32_t mqtt_msg_id = 0;

/** MQTT 连接状态：-1=连接中 0=断开 1=已连接（供 password.c 轮询更新 LCD） */
volatile int g_mqtt_connected = -1;

#define ONENET_POST_TOPIC "$sys/85mL5Zf9EC/ESP32/thing/property/post"

//==============================================================================
// MQTT 数据上报接口（供 password.c 等调用）
//==============================================================================

/** 上报当前的正确密码（OneNet 物模型 password 为 int32） */
void mqtt_publish_password(const char *pwd)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "mqtt_publish_password: client not initialized");
        return;
    }
    char data[256];
    int pwd_int = atoi(pwd);
    uint32_t mid = ++mqtt_msg_id;
    snprintf(data, sizeof(data),
        "{\"id\":\"%lu\",\"version\":\"1.0\",\"params\":{\"password\":{\"value\":%d}}}",
        (unsigned long)mid, pwd_int);
    int msg_id = esp_mqtt_client_publish(mqtt_client, ONENET_POST_TOPIC, data, 0, 1, 0);
    ESP_LOGI(TAG, "Published password=%d, msg_id=%d", pwd_int, msg_id);
}

/** 上报密码输入错误（发送 1） */
void mqtt_publish_wrong_pwd(void)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "mqtt_publish_wrong_pwd: client not initialized");
        return;
    }
    char data[128];
    uint32_t mid = ++mqtt_msg_id;
    snprintf(data, sizeof(data),
        "{\"id\":\"%lu\",\"version\":\"1.0\",\"params\":{\"alarm\":{\"value\":1}}}",
        (unsigned long)mid);
    int msg_id = esp_mqtt_client_publish(mqtt_client, ONENET_POST_TOPIC, data, 0, 1, 0);
    ESP_LOGI(TAG, "Published wrong_pwd, msg_id=%d", msg_id);
}

/** 上报门锁状态：1=打开，0=关闭 */
void mqtt_publish_lock_status(int locked)
{
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "mqtt_publish_lock_status: client not initialized");
        return;
    }
    char data[128];
    uint32_t mid = ++mqtt_msg_id;
    snprintf(data, sizeof(data),
        "{\"id\":\"%lu\",\"version\":\"1.0\",\"params\":{\"LockState\":{\"value\":%d}}}",
        (unsigned long)mid, locked ? 1 : 0);
    int msg_id = esp_mqtt_client_publish(mqtt_client, ONENET_POST_TOPIC, data, 0, 1, 0);
    ESP_LOGI(TAG, "Published lock_status=%d, msg_id=%d", locked ? 1 : 0, msg_id);
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        g_mqtt_connected = 1;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        g_mqtt_connected = 0;
        mqtt_session_inited = false;   // 重新连接后重新初始化
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        //如果主题是 $sys/85mL5Zf9EC/ESP32/thing/property/set，那么回复收到
        if (event->topic_len == strlen("$sys/85mL5Zf9EC/ESP32/thing/property/set") &&
            memcmp(event->topic, "$sys/85mL5Zf9EC/ESP32/thing/property/set", event->topic_len) == 0) {
            cJSON *root = cJSON_Parse(event->data);
            if (root) {
                cJSON *id = cJSON_GetObjectItem(root, "id");
                // 回复格式: {"id":"<id>","code":200,"msg":"success"}，id要根据收到id来设置，通过json解析收到的id
                if (cJSON_IsString(id) && (id->valuestring != NULL)) {
                    char reply_data[128];
                    snprintf(reply_data, sizeof(reply_data), "{\"id\":\"%s\",\"code\":200,\"msg\":\"success\"}", id->valuestring);
                    msg_id = esp_mqtt_client_publish(client, "$sys/85mL5Zf9EC/ESP32/thing/property/set_reply", reply_data, 0, 1, 0);
                    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
                }
                cJSON_Delete(root);
            } else {
                ESP_LOGE(TAG, "Failed to parse JSON data");
            }
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_init_session(void)
{
    if (mqtt_client == NULL || g_mqtt_connected != 1 || mqtt_session_inited) {
        return;
    }
    mqtt_session_inited = true;

    int msg_id;

    msg_id = esp_mqtt_client_subscribe(mqtt_client, "$sys/85mL5Zf9EC/ESP32/thing/property/post/reply", 0);
    ESP_LOGI(TAG, "subscribed post/reply, msg_id=%d", msg_id);

    msg_id = esp_mqtt_client_subscribe(mqtt_client, "$sys/85mL5Zf9EC/ESP32/thing/property/set", 0);
    ESP_LOGI(TAG, "subscribed set, msg_id=%d", msg_id);

    // 连接成功后发送一次 password（测试用，按 int32 发送）
    uint32_t mid = ++mqtt_msg_id;
    char boot_data[128];
    snprintf(boot_data, sizeof(boot_data),
        "{\"id\":\"%lu\",\"version\":\"1.0\",\"params\":{\"password\":{\"value\":1234}}}",
        (unsigned long)mid);
    msg_id = esp_mqtt_client_publish(mqtt_client, ONENET_POST_TOPIC, boot_data, 0, 1, 0);
    ESP_LOGI(TAG, "boot publish password=1234, msg_id=%d", msg_id);
}

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://mqtts.heclouds.com:1883",
        .credentials.username = "85mL5Zf9EC", //OneNet product ID
        .credentials.client_id = "ESP32",  //OneNet device name
        .credentials.authentication.password = // token
            "version=2018-10-31&res=products%2F85mL5Zf9EC%2Fdevices%2FESP32&et=2065683892&method=md5&sign=4REePqVYZpUJkGWtgv0hnQ%3D%3D",
        .session.keepalive = 60,              // 心跳间隔60秒
        .session.disable_clean_session = false,
        .network.timeout_ms = 3000,            // 网络超时3秒
        .network.reconnect_timeout_ms = 3000,  // 断线后3秒重连
        .buffer.size = 2048,                   // 收发缓冲区2KB
        .task.stack_size = 8192,               // MQTT任务栈8KB（防栈溢出）
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

// app_main() 已合并到 main.c 中