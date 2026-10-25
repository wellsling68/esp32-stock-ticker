#include <stdio.h>

//wifi driver
#include "esp_log.h"
#include "wifi_sta.h"
#include "display_init.h"

static const char *TAG = "main";

// static void fetch_task(void *arg)
// {
//     if (http_get("https://httpbin.org/get")) {
//         ESP_LOGI(TAG, "\nBody: %s", s_body);
//         parse_response();
//     }
//     vTaskDelete(NULL);
// }

// static void fill_rect(esp_lcd_panel_handle_t panel, int x, int y, int w, int h, uint16_t color)
// {
//     static uint16_t row[480];                    // big enough for either axis
//     for (int i = 0; i < w; i++) row[i] = color;
//     for (int j = 0; j < h; j++) {
//         esp_lcd_panel_draw_bitmap(panel, x, y + j, x + w, y + j + 1, row);
//     }
// }

void app_main(void)
{
    ESP_LOGI(TAG, "Flash Successful!");
    ESP_LOGI(TAG, "Connecting to WIFI");
    wifi_connect();

    // esp_lcd_panel_handle_t panel;
    // display_init(&panel);
    // xTaskCreate(fetch_task, "fetch", 8192, NULL, 5, NULL);

    // // in app_main, after display_init(&panel):
    // fill_rect(panel, 0, 0, 320, 480, 0x0000);        // black background
    // fill_rect(panel, 0,   0,   80, 80, 0xFFFF);      // BIG square at x=0,   y=0
    // fill_rect(panel, 280, 0,   40, 40, 0xFFFF);      // MEDIUM square at x=280, y=0
    // fill_rect(panel, 0,   440, 20, 40, 0xFFFF);      // TALL thin rectangle at x=0, y=440
}
