#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "display.h"
#include "esp_lvgl_port.h"

// static lv_obj_t *s_symbol_label;
// static lv_obj_t *s_price_label;
// static lv_obj_t *s_change_label;

static lv_obj_t *s_symbol, *s_price, *s_change;

static lv_obj_t *make_label(lv_obj_t *parent, const lv_font_t *font)
{
    lv_obj_t *l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_color(l, lv_color_white(), 0);
    return l;
}

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
        // s_symbol_label = lv_label_create(scr);
        // lv_obj_align(s_symbol_label, LV_ALIGN_TOP_MID, 0, 40);
        // s_price_label = lv_label_create(scr);
        // lv_obj_align(s_price_label, LV_ALIGN_CENTER, 0, 0);
        // s_change_label = lv_label_create(scr);
        // lv_obj_align(s_change_label, LV_ALIGN_CENTER, 0, 40);
        
        lv_obj_set_style_bg_color(scr, lv_color_hex(0x13151A), 0);

        lv_obj_t *row = lv_obj_create(scr);
        lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_style_pad_column(row, 12, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_align(row, LV_ALIGN_TOP_LEFT, 10, 10);

        s_symbol = make_label(row, &lv_font_montserrat_28);
        s_price  = make_label(row, &lv_font_montserrat_28);
        s_change = make_label(row, &lv_font_montserrat_20);


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
    char p[16], c[16];
    snprintf(p, sizeof(p), "%.2f", price);
    snprintf(c, sizeof(c), "%+.2f%%", change_pct);

    if (lvgl_port_lock(0)) {
        lv_label_set_text(s_symbol, symbol);
        lv_label_set_text(s_price, p);
        lv_label_set_text(s_change, c);
        lv_obj_set_style_text_color(s_change,
            change_pct >= 0 ? lv_color_hex(0x2ECC71) : lv_color_hex(0xFF4D4F), 0);
        lvgl_port_unlock();
    }
}