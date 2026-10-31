#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "display.h"
#include "esp_lvgl_port.h"



static lv_obj_t *s_symbol_label;
static lv_obj_t *s_price_label;
static lv_obj_t *s_change_label;

esp_err_t lvgl_init(void)
{
    // connection to display
    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_handle_t panel;
    ESP_ERROR_CHECK(display_init(&io, &panel));

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io,
        .panel_handle = panel,
        .buffer_size = DISPLAY_H_RES * 40,
        .double_buffer = true,
        .hres = DISPLAY_H_RES,
        .vres = DISPLAY_V_RES,
        .rotation = {
            .swap_xy  = DISPLAY_SWAP_XY
            // .mirror_x = DISPLAY_MIRROR_X,
            // .mirror_y = DISPLAY_MIRROR_Y,
        },
        .monochrome = false,
        // .rotation = { .swap_xy = false, .mirror_x = false, .mirror_y = false },
        .flags = { .buff_dma = true, .swap_bytes = true },
    };
    lv_display_t *disp = lvgl_port_add_disp(&disp_cfg);
    if (!disp) return ESP_FAIL;

    //writing to display
    if (lvgl_port_lock(0)) {
        lv_obj_t *scr = lv_screen_active();
        s_symbol_label = lv_label_create(scr);
        lv_obj_align(s_symbol_label, LV_ALIGN_TOP_MID, 0, 40);
        s_price_label = lv_label_create(scr);
        lv_obj_align(s_price_label, LV_ALIGN_CENTER, 0, 0);
        s_change_label = lv_label_create(scr);
        lv_obj_align(s_change_label, LV_ALIGN_CENTER, 0, 40);

        // --- temporary: corner orientation test ---
        static const struct { lv_align_t align; uint32_t color; } corners[] = {
            { LV_ALIGN_TOP_LEFT,     0xFFFF00 },  // yellow
            { LV_ALIGN_TOP_RIGHT,    0x00FFFF },  // cyan
            { LV_ALIGN_BOTTOM_LEFT,  0xFF00FF },  // magenta  
            { LV_ALIGN_BOTTOM_RIGHT, 0xFFFFFF },  // white
        };
        for (int i = 0; i < 4; i++) {
            lv_obj_t *sq = lv_obj_create(scr);
            lv_obj_set_size(sq, 20, 20);
            lv_obj_set_style_bg_color(sq, lv_color_hex(corners[i].color), 0);
            lv_obj_align(sq, corners[i].align, 0, 0);
        }
        // --- end test ---

        lvgl_port_unlock();
    }
    return ESP_OK;
}

void ui_show_quote(const char *symbol, float price, float change_pct)
{
    char price_buf[16], change_buf[16];
    snprintf(price_buf, sizeof(price_buf), "%.2f", price);
    snprintf(change_buf, sizeof(change_buf), "%+.2f%%", change_pct);

    if (lvgl_port_lock(0)) {
        lv_label_set_text(s_symbol_label, symbol);
        lv_label_set_text(s_price_label, price_buf);
        lv_label_set_text(s_change_label, change_buf);
        lv_obj_set_style_text_color(s_change_label,
            change_pct >= 0 ? lv_color_hex(0x00C853) : lv_color_hex(0xD50000), 0);
        lvgl_port_unlock();
    }
}