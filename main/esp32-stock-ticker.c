#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "wifi-sta.h"
// #include "display.h"
#include "api-fetch.h"
#include "ui.h"
#include "input.h"

static const char *TAG = "main";

// static void fill_rect(esp_lcd_panel_handle_t panel, int x, int y, int w, int h, uint16_t color)
// {
//     static uint16_t row[480];                    // big enough for either axis
//     for (int i = 0; i < w; i++) row[i] = color;
//     for (int j = 0; j < h; j ++) {
//         esp_lcd_panel_draw_bitmap(panel, x, y + j, x + w, y + j + 1, row);
//     }
// }

void finnhub_fetch(void *arg)
{
    /* Reference:
     *
     *  typedef struct {
     *      float current;        // c
     *      float change;         // d
     *      float percent_change; // dp
     *      float high;           // h
     *      float low;            // l
     *      float open;           // o
     *      float prev_close;     // pc
     *      uint32_t timestamp;   // t
     *  } finnhub_quote_t;
    */

    finnhub_quote_t q;
    while (1) {
        if (finnhub_get_quote("AAPL", &q) == ESP_OK) {
            ESP_LOGI("main", "AAPL $%.2f (%.2f%%)", q.current, q.percent_change);
            ui_show_quote("AAPL", q.current, q.percent_change);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Flash Successful!");
    ESP_LOGI(TAG, "Connecting to WIFI");
    wifi_connect();

    // esp_lcd_panel_handle_t panel;
    // esp_lcd_panel_io_handle_t io;

    // display_init(&io, &panel);
    // ESP_LOGI(TAG, "Display Initialized");
    ESP_ERROR_CHECK(lvgl_init());

    xTaskCreate(finnhub_fetch, "ticker", 8192, NULL, 5, NULL);

    // in app_main, after display_init(&panel):
    // fill_rect(panel, 0, 0, 320, 480, 0x0000);        // black background
    // fill_rect(panel, 0,   0,   80, 80, 0xFFFF);      // BIG square at x=0,   y=0
    // fill_rect(panel, 280, 0,   40, 40, 0xFFFF);      // MEDIUM square at x=280, y=0
    // fill_rect(panel, 0,   440, 20, 40, 0xFFFF);      // TALL thin rectangle at x=0, y=440    
    QueueHandle_t q = input_init();
    input_evt_t e;
    while (xQueueReceive(q, &e, portMAX_DELAY)) {
        ESP_LOGI("main", "event: %s",
                e == INPUT_EVT_CW ? "CW" : e == INPUT_EVT_CCW ? "CCW" : "PRESS");
    }
}
