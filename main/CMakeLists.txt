idf_component_register(SRCS "main.c" "ticker.c"
                    INCLUDE_DIRS "."
                    REQUIRES wifi-sta display api-fetch ui input
                    )
