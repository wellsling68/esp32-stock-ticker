#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

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

static const char *SYMBOLS[] = { "META", "AAPL", "PLTR", "TSM", "GOOG", "NVDA" };

// lenght of array calculated by dividing size of array / size of item
#define NUM_SYMBOLS  (sizeof(SYMBOLS) / sizeof(SYMBOLS[0]))
#define REFRESH_MS   2000

static finnhub_quote_t   s_cache[NUM_SYMBOLS];
static bool              s_valid[NUM_SYMBOLS];
static SemaphoreHandle_t s_lock;           // protects s_cache / s_valid
static QueueHandle_t     s_req_q;          // length 1: "fetch this symbol index"
static volatile size_t   s_current = 0;    // symbol currently on screen

static void show_cached(size_t i)
{
    finnhub_quote_t q;
    bool ok;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    q = s_cache[i];
    ok = s_valid[i];
    xSemaphoreGive(s_lock);
    if (ok) ui_show_quote(SYMBOLS[i], q.current, q.percent_change);
}

static void fetch_symbol(size_t i)
{
    finnhub_quote_t q;
    if (finnhub_get_quote(SYMBOLS[i], &q) != ESP_OK) return;

    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_cache[i] = q;
    s_valid[i] = true;
    xSemaphoreGive(s_lock);

    if (i == s_current) {                  // only draw it if it's still the one on screen
        ui_show_quote(SYMBOLS[i], q.current, q.percent_change);
    }
}

// Does all the slow network work
static void fetch_task(void *arg)
{
    size_t warm = 0;                      // next symbol to pre-load
    while (1) {
        size_t req;
        // While warming up, don't block: grab a user request if there is one,
        // otherwise pre-load the next symbol. After warm-up, sleep until asked.
        TickType_t wait = (warm < NUM_SYMBOLS) ? 0 : portMAX_DELAY;
        if (xQueueReceive(s_req_q, &req, wait) != pdTRUE) {
            req = warm++;
        }
        fetch_symbol(req);
    }
}

// Handles input instantly and schedules refreshes
static void ticker_task(void *arg)
{
    QueueHandle_t input_q = (QueueHandle_t)arg;
    input_evt_t e;

    while (1) {
        // Wait up to 2 s for input; the wait restarts after every event
        if (xQueueReceive(input_q, &e, pdMS_TO_TICKS(REFRESH_MS)) == pdTRUE) {
            size_t idx = s_current;
            if (e == INPUT_EVT_CW)       idx = (idx + 1) % NUM_SYMBOLS;
            else if (e == INPUT_EVT_CCW) idx = (idx + NUM_SYMBOLS - 1) % NUM_SYMBOLS;
            s_current = idx;
            show_cached(idx);             // instant
        }
        // Switch or timeout: ask for a fresh quote of whatever is on screen
        size_t cur = s_current;
        xQueueOverwrite(s_req_q, &cur);   // latest request wins, no pile-up
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

    s_lock  = xSemaphoreCreateMutex();
    s_req_q = xQueueCreate(1, sizeof(size_t));
    QueueHandle_t input_q = input_init();

    xTaskCreate(fetch_task,  "fetch",  8192, NULL,    5, 0);
    xTaskCreate(ticker_task, "ticker", 4096, input_q, 5, NULL);

    // xTaskCreate(finnhub_fetch, "ticker", 8192, NULL, 5, NULL);

    // in app_main, after display_init(&panel):
    // fill_rect(panel, 0, 0, 320, 480, 0x0000);        // black background
    // fill_rect(panel, 0,   0,   80, 80, 0xFFFF);      // BIG square at x=0,   y=0
    // fill_rect(panel, 280, 0,   40, 40, 0xFFFF);      // MEDIUM square at x=280, y=0
    // fill_rect(panel, 0,   440, 20, 40, 0xFFFF);      // TALL thin rectangle at x=0, y=440    
    // QueueHandle_t q = input_init();
    // input_evt_t e;
    // while (xQueueReceive(q, &e, portMAX_DELAY)) {
    //     e == INPUT_EVT_CW ? "CW" : e == INPUT_EVT_CCW ? "CCW" : "PRESS";
    //     ESP_LOGI("main", "event: %s", e);
    
    // }
}
