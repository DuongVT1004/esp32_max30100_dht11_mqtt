#ifndef __APP_MQTT_H
#define __APP_MQTT_H

#include <stdbool.h>
#include "mqtt_client.h"

typedef struct {
    char buf[256];
    int offset;
} json_gen_test_result_t;

typedef enum {
    max30100_id = 1,
    hum_tem_id,
} id_typedef;


void mqtt_app_start(void);
void json_gen_test(json_gen_test_result_t *result, char *key1, bool value1, char *key2, int value2, char *key3, char *value3);

#endif