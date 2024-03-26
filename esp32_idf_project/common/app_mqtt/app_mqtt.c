/* MQTT Mutual Authentication Example

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
//#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
//#include "mqtt_client.h"
#include "json_generator.h"
#include "json_parser.h"
#include "app_mqtt.h"
#include "max30100.h"
#include <stdlib.h>

static const char *TAG = "MQTT_APP";

extern id_typedef id;

extern max30100_data_t data_max30100;
//char data[100] = {0};
esp_mqtt_client_handle_t client;

// extern const uint8_t client_cert_pem_start[] asm("_binary_client_crt_start");
// extern const uint8_t client_cert_pem_end[] asm("_binary_client_crt_end");
// extern const uint8_t client_key_pem_start[] asm("_binary_client_key_start");
// extern const uint8_t client_key_pem_end[] asm("_binary_client_key_end");


static void flush_str(char *buf, void *priv)
{
    json_gen_test_result_t *result = (json_gen_test_result_t *)priv;
    if (result) {
        if (strlen(buf) > sizeof(result->buf) - result->offset) {
            printf("Result Buffer too small\r\n");
            return;
        }
        memcpy(result->buf + result->offset, buf, strlen(buf));
        result->offset += strlen(buf);
    }
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    //sprintf(data, "BPM: %f \n SpO2: %f \n", data_max30100.heart_bpm, data_max30100.spO2);
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/Thuy/Duong", 0);
            msg_id = esp_mqtt_client_subscribe(client, "/Weather/Hum_Tem", 0);
            ESP_LOGI(TAG, "sent subscribe successful");
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            if(id == max30100_id)
            {
                ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
                msg_id = esp_mqtt_client_publish(client, "/Thuy/Duong", "BPM, spO2 Information", 0, 0, 0);
                ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            }
            else if(id == hum_tem_id)
            {
                ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
                msg_id = esp_mqtt_client_publish(client, "/Weather/Hum_Tem", "Humidity and Temperature Information", 0, 0, 0);
                ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            }
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
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://test.mosquitto.org:1883",
        .event_handle = mqtt_event_handler,
        // .client_cert_pem = (const char *)client_cert_pem_start,
        // .client_key_pem = (const char *)client_key_pem_start,
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

void json_gen_test(json_gen_test_result_t *result, char *key1, bool value1, char *key2, int value2, char *key3, char *value3)
{
    char buf[20];
    memset(result, 0, sizeof(json_gen_test_result_t));
	json_gen_str_t jstr;
	json_gen_str_start(&jstr, buf, sizeof(buf), flush_str, result);
    json_gen_start_object(&jstr);
    json_gen_obj_set_bool(&jstr, key1, value1);
    json_gen_obj_set_int(&jstr, key2, value2);
    json_gen_obj_set_string(&jstr, key3, value3);
    json_gen_end_object(&jstr);
    json_gen_str_end(&jstr);
    printf("Result: %s\n", result->buf);
}


