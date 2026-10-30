idf_component_register(SRCS "esp32-stock-ticker.c"
                    INCLUDE_DIRS "."
                    REQUIRES wifi_sta display api_fetch
                    )
