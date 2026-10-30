#include "esp_lvgl_port.h"
#include "display_init.h"

lv_display_t *ui_init(void)
{
    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_handle_t panel;
    ESP_ERROR_CHECK(display_init(&io, &panel));

    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io,
        .panel_handle = panel,
        .buffer_size = 320 * 40,      // pixels, matches your max_transfer_sz
        .double_buffer = true,
        .hres = 320,
        .vres = 480,
        .monochrome = false,
        .rotation = { .swap_xy = false, .mirror_x = false, .mirror_y = false },
        .flags = { .buff_dma = true, .swap_bytes = true },
    };
    return lvgl_port_add_disp(&disp_cfg);
}