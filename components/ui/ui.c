// typedef struct {
//     float current;        // c
//     float change;         // d
//     float percent_change; // dp
//     float high;           // h
//     float low;            // l
//     float open;           // o
//     float prev_close;     // pc
//     uint32_t timestamp;   // t
// } finnhub_quote_t;

#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "ui-quote-list.h"
#include "display.h"
#include "esp_lvgl_port.h"

lv_obj_t *scr_list, *scr_detail, *scr_error;

//static void make_detailed_quote
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
        scr_list = lv_screen_active();
        quote_list_create(scr_list);

        // scr_detail = lv_obj_create(NULL);
        // lv_obj_set_style_bg_color(scr_detail, lv_color_hex(0x13151A), 0);
        // lv_obj_set_style_pad_all(scr_detail, 8, 0);
        // make_list_item(scr_detail, &detail_quote);


        // scr_error = lv_obj_create(NULL);    // NULL parent = a new screen
        // lv_obj_set_style_bg_color(scr_error, lv_color_hex(0x13151A), 0);
        // err_label = make_label(scr_error, &lv_font_montserrat_20);
        // lv_obj_center(err_label);

        // --- temporary: corner orientation test ---
        // static const struct { lv_align_t align; uint32_t color; } corners[] = {
        //     { LV_ALIGN_TOP_LEFT,     0xFFFF00 },  // yellow
        //     { LV_ALIGN_TOP_RIGHT,    0x00FFFF },  // cyan
        //     { LV_ALIGN_BOTTOM_LEFT,  0xFF00FF },  // magenta  
        //     { LV_ALIGN_BOTTOM_RIGHT, 0xFFFFFF },  // white
        // };
        // for (int i = 0; i < 4; i++) {
        //     lv_obj_t *sq = lv_obj_create(NULL);
        //     lv_obj_set_size(sq, 20, 20);
        //     lv_obj_set_style_bg_color(sq, lv_color_hex(corners[i].color), 0);
        //     lv_obj_align(sq, corners[i].align, 0, 0);
        // }
        // --- end test ---

        lvgl_port_unlock();
    }
    return ESP_OK;
}

// void ui_show_quote(const char *symbol, float price, float chg)
// {
//     char p[16], c[16];
//     snprintf(p, sizeof(p), "%.2f", price);
//     snprintf(c, sizeof(c), "%+.2f", chg);

//     if (lvgl_port_lock(0)) {
//         lv_label_set_text(detail_quote.sym, symbol);
//         lv_label_set_text(detail_quote.price, p);
//         lv_label_set_text(detail_quote.pct, c);
//         lv_obj_set_style_text_color(detail_quote.pct,
//             chg >= 0 ? lv_color_hex(0x2ECC71) : lv_color_hex(0xFF4D4F), 0);
        
//         if (lv_screen_active() != scr_detail) lv_screen_load(scr_detail);
//         lvgl_port_unlock();
//     }
// }

void ui_show_list(const char *const *symbols, const char *const *names,
                  const finnhub_quote_t *q, const bool *valid,
                  size_t count, size_t selected)
{
    if (!lvgl_port_lock(0)) return;
    quote_list_update(symbols, names, q, valid, count, selected);
    if (lv_screen_active() != scr_list) lv_screen_load(scr_list);
    lvgl_port_unlock();
}

// void ui_show_error(const char *msg)
// {
//     ESP_LOGI("ui", "ERROR: %s", msg);
//     if (lvgl_port_lock(0)) {
//         lv_label_set_text(err_label, msg);
//         lv_screen_load(scr_error);
//         lvgl_port_unlock();
//     }
// }