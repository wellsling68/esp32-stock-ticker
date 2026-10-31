#pragma once
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#define DISPLAY_H_RES 480 // 320 vertical  
#define DISPLAY_V_RES 320 // 480 vertical

#define DISPLAY_SWAP_XY  true
// #define DISPLAY_MIRROR_X true
// #define DISPLAY_MIRROR_Y false

esp_err_t display_init(esp_lcd_panel_io_handle_t *out_io, esp_lcd_panel_handle_t *out_panel);