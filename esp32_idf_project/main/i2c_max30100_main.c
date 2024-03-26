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
#include "freertos/timers.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

#include "max30100.h"
#include "app_mqtt.h"
#include "app_config.h"
#include "dht11.h"

#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_FRQ 100000
#define I2C_PORT I2C_NUM_0

static const char *TAGM = "I2C_MQTT";

TimerHandle_t xTimers;
max30100_config_t max30100 = {};
max30100_data_t data_max30100 = {};
esp_mqtt_client_handle_t client;
char data[100] = {0};
char hum_tem_data[50] = {0};
float bpm, spo2;
uint8_t flag_Timer = 1;
static uint8_t hum_tem_flag = 0;

id_typedef id = max30100_id;

uint8_t flag = 1;
struct dht11_reading dht11_cur_reading, dht11_last_reading;

esp_err_t i2c_master_init(i2c_port_t i2c_port){
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_SDA;
    conf.scl_io_num = I2C_SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FRQ;
    i2c_param_config(i2c_port, &conf);
    return i2c_driver_install(i2c_port, I2C_MODE_MASTER, 0, 0, 0);
}

void get_bpm(void* param) {
    printf("MAX30100 Test\n");
    while(true) {
        //Update sensor, saving to "result"
        ESP_ERROR_CHECK(max30100_update(&max30100, &data_max30100));
        if(data_max30100.pulse_detected) {
            printf("BEAT\n");
            printf("BPM: %.2f | SpO2: %.2f%%\n", data_max30100.heart_bpm, data_max30100.spO2);
            bpm = data_max30100.heart_bpm;
            spo2 = data_max30100.spO2;
            flag = 0;
            if(flag_Timer == 1)
            {
                xTimerStart(xTimers, 0);
                flag_Timer = 0;
            }
        }
        else{
            flag = 1;
        }
        //Update rate: 100Hz
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
    
}

static void vTimerCallback( TimerHandle_t xTimer )
{
    uint32_t ID;

    /* Optionally do something if the pxTimer parameter is NULL. */
    configASSERT( xTimer );

    /* The number of times this timer has expired is saved as the
    timer's ID.  Obtain the count. */
    
    hum_tem_flag++;
    ID = ( uint32_t ) pvTimerGetTimerID( xTimer );
    if(ID == 0)
    {
        if(flag == 1)
        {
            sprintf(data, "BPM: %.2f \nSpO2: %.2f \n", bpm, spo2);
            esp_mqtt_client_publish(client, "/Thuy/Duong", data, 0, 0, 0);
            id = max30100_id;
        }
        else if(flag == 0)
        {
            sprintf(data, "BPM: %f \n SpO2: %f \n", data_max30100.heart_bpm, data_max30100.spO2);
            esp_mqtt_client_publish(client, "/Thuy/Duong", data, 0, 0, 0);
        }
    }
    if(hum_tem_flag == 5)
    {
        sprintf(hum_tem_data, "Temperature: %d \n Humidity: %d \n", dht11_last_reading.temperature, dht11_last_reading.humidity);
        esp_mqtt_client_publish(client, "/Weather/Hum_Tem", hum_tem_data, 0, 0, 0);
        id = hum_tem_id;
        hum_tem_flag = 0;
    }
    //xTimerStop(xTimers, 0);
}

void dht11_task(void *param)
{
    while(1)
    {
        dht11_cur_reading = DHT11_read();
        if(dht11_cur_reading.status == 0)
        {
            dht11_last_reading = dht11_cur_reading;
            printf("temp: %d\n", dht11_last_reading.temperature);
            printf("humi: %d\n", dht11_last_reading.humidity);
        }
        else{
            printf("read fail\n");
        }
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}

void app_main()
{
    ESP_LOGI(TAGM, "[APP] Startup..");
    ESP_LOGI(TAGM, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAGM, "[APP] IDF version: %s", esp_get_idf_version());


    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //Init I2C_NUM_0
    ESP_ERROR_CHECK(i2c_master_init(I2C_PORT));
    //Init sensor at I2C_NUM_0
   ESP_ERROR_CHECK(max30100_init( &max30100, I2C_PORT,
                   MAX30100_DEFAULT_OPERATING_MODE,
                   MAX30100_DEFAULT_SAMPLING_RATE,
                   MAX30100_DEFAULT_LED_PULSE_WIDTH,
                   MAX30100_DEFAULT_IR_LED_CURRENT,
                   MAX30100_DEFAULT_START_RED_LED_CURRENT,
                   MAX30100_DEFAULT_MEAN_FILTER_SIZE,
                   MAX30100_DEFAULT_PULSE_BPM_SAMPLE_SIZE,
                   true, false ));

    //config wifi
    app_config();
    
    // start matt
    mqtt_app_start();

    //init dht11
    DHT11_init(GPIO_NUM_4);
    
    xTaskCreate(get_bpm, "Get BPM", 8192, NULL, 1, NULL);
    xTaskCreate(dht11_task, "DHT11_Task", 8192, NULL, 1, NULL);
    xTimers = xTimerCreate("TimerForTimeOut", 2000/portTICK_RATE_MS, pdTRUE, (void *)0, vTimerCallback);
    xTimerStart(xTimers, 0);
   
    
    
}