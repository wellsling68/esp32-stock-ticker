idf_component_register(SRCS "esp32_stock_ticker.c"
                    INCLUDE_DIRS "."
                    REQUIRES wifi_sta display_init api_fetch
                    )
