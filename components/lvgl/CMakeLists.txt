idf_component_register(
    SRCS "lvgl.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_lcd
    PRIV_REQUIRES driver
)
