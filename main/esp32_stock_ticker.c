#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//wifi driver
#include "esp_log.h"
#include "wifi_sta.h"
#include "display_init.h"
#include "api_fetch.h"

static const char *TAG = "main";

static void fill_rect(esp_lcd_panel_handle_t panel, int x, int y, int w, int h, uint16_t color)
{
    static uint16_t row[480];                    // big enough for either axis
    for (int i = 0; i < w; i++) row[i] = color;
    for (int j = 0; j < h; j++) {
        esp_lcd_panel_draw_bitmap(panel, x, y + j, x + w, y + j + 1, row);
    }
}

void finnhub_fetch(void *arg)
{
    finnhub_quote_t q;
    while (1) {
        if (finnhub_get_quote("AAPL", &q) == ESP_OK) {
            ESP_LOGI("main", "AAPL $%.2f (%.2f%%)", q.current, q.percent_change);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Flash Successful!");
    ESP_LOGI(TAG, "Connecting to WIFI");
    wifi_connect();

    esp_lcd_panel_handle_t panel;
    display_init(&panel);

    // in app_main, after display_init(&panel):
    fill_rect(panel, 0, 0, 320, 480, 0x0000);        // black background
    fill_rect(panel, 0,   0,   80, 80, 0xFFFF);      // BIG square at x=0,   y=0
    fill_rect(panel, 280, 0,   40, 40, 0xFFFF);      // MEDIUM square at x=280, y=0
    fill_rect(panel, 0,   440, 20, 40, 0xFFFF);      // TALL thin rectangle at x=0, y=440

    xTaskCreate(finnhub_fetch, "ticker", 8192, NULL, 5, NULL);
}
