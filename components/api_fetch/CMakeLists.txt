idf_component_register(
    SRCS "api_fetch.c"
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_http_client esp-tls espressif__cjson
)
