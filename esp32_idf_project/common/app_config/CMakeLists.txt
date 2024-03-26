set(app_src app_config.c )
set(pri_req nvs_flash http_server_app)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})

