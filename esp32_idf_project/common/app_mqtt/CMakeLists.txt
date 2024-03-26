set(app_src app_mqtt.c)
set(pri_req nvs_flash mqtt json_parser json_generator app_max30100)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})

