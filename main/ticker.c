#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "api-fetch.h"
#include "ui.h"
#include "input.h"

static const char *SYMBOLS[] = { "META", "AAPL", "PLTR", "TSM", "PLBL", "NVDA" };
static const char *NAMES[] = { "Meta Platforms", "Apple Inc.", "Palantir", "TSMC", "<PLBL name>", "NVIDIA" };
#define NUM_SYMBOLS        (sizeof(SYMBOLS) / sizeof(SYMBOLS[0]))

#define DETAIL_REFRESH_MS  2000   // expanded view
#define LIST_REFRESH_MS    5000   // list: one symbol per tick, round-robin
#define ERROR_RETRY_MS     5000
#define FAIL_LIMIT         3      // consecutive failures before the error screen

typedef enum { SCREEN_LIST, SCREEN_DETAIL, SCREEN_ERROR } screen_t;

static volatile screen_t s_screen = SCREEN_LIST;
static volatile size_t   s_sel = 0;          // highlighted (list) / shown (detail) symbol
static volatile int      s_fail_count = 0;
static volatile bool     s_dirty = false;    // fetch task -> ticker task: "redraw please"

static finnhub_quote_t   s_cache[NUM_SYMBOLS];
static bool              s_valid[NUM_SYMBOLS];
static SemaphoreHandle_t s_lock;
static QueueHandle_t     s_req_q;

/* ---------- drawing (ticker task only) ---------- */

static void snapshot(finnhub_quote_t *q, bool *v)
{
    xSemaphoreTake(s_lock, portMAX_DELAY);
    memcpy(q, s_cache, sizeof(s_cache));
    memcpy(v, s_valid, sizeof(s_valid));
    xSemaphoreGive(s_lock);
}

static void redraw(void)
{
    finnhub_quote_t q[NUM_SYMBOLS];
    bool v[NUM_SYMBOLS];

    switch (s_screen) {
        case SCREEN_LIST:
            snapshot(q, v);
            ui_show_list(SYMBOLS, NAMES, q, v, NUM_SYMBOLS, s_sel);
            break;
        case SCREEN_DETAIL:
            snapshot(q, v);
            // if (v[s_sel]) ui_show_quote(SYMBOLS[s_sel], q[s_sel].current, q[s_sel].change);
            break;
        case SCREEN_ERROR:
            // ui_show_error("WiFi or API failure - retrying");
            break;
    }
}

/* ---------- fetch side ---------- */

static void request_fetch(size_t i) { xQueueOverwrite(s_req_q, &i); }

static void fetch_symbol(size_t i)
{
    finnhub_quote_t q;
    if (finnhub_get_quote(SYMBOLS[i], &q) != ESP_OK) {
        s_fail_count++;
        return;
    }
    s_fail_count = 0;

    xSemaphoreTake(s_lock, portMAX_DELAY);
    s_cache[i] = q;
    s_valid[i] = true;
    xSemaphoreGive(s_lock);

    if (s_screen == SCREEN_LIST || i == s_sel) s_dirty = true;
}

void fetch_task(void *arg)
{
    size_t warm = 0;   // startup: pre-load every symbol, but serve requests first
    while (1) {
        size_t req;
        TickType_t wait = (warm < NUM_SYMBOLS) ? 0 : portMAX_DELAY;
        if (xQueueReceive(s_req_q, &req, wait) != pdTRUE) req = warm++;
        fetch_symbol(req);
    }
}

/* ---------- state machine ---------- */

static size_t next_sym(size_t i, bool cw)
{
    return cw ? (i + 1) % NUM_SYMBOLS : (i + NUM_SYMBOLS - 1) % NUM_SYMBOLS;
}

static void handle_event(input_evt_t e, TickType_t *next_req)
{
    bool turn = (e == INPUT_EVT_CW || e == INPUT_EVT_CCW);
    bool cw   = (e == INPUT_EVT_CW);

    switch (s_screen) {
    case SCREEN_LIST:
        if (turn) {
            s_sel = next_sym(s_sel, cw);
            redraw();
        } else if (e == INPUT_EVT_PRESS) {
            s_screen = SCREEN_DETAIL;
            redraw();
            *next_req = xTaskGetTickCount();     // fetch right away
        }
        break;

    case SCREEN_DETAIL:
        if (turn) {
            s_sel = next_sym(s_sel, cw);
            redraw();                            // instant, from cache
            *next_req = xTaskGetTickCount();     // fetch now; 2 s timer restarts after
        } else if (e == INPUT_EVT_LONG) {
            s_screen = SCREEN_LIST;
            redraw();
        }
        break;

    case SCREEN_ERROR:
        if (e == INPUT_EVT_PRESS) *next_req = xTaskGetTickCount();   // retry now
        break;
    }
}

void ticker_task(void *arg)
{
    QueueHandle_t input_q = (QueueHandle_t)arg;
    input_evt_t e;
    TickType_t next_req = xTaskGetTickCount();
    size_t rr = 0;                               // round-robin index for list refresh

    while (1) {
        if (xQueueReceive(input_q, &e, pdMS_TO_TICKS(50)) == pdTRUE) {
            handle_event(e, &next_req);
        }
        TickType_t now = xTaskGetTickCount();

        // enter / leave the error screen based on fetch results
        if (s_screen != SCREEN_ERROR && s_fail_count >= FAIL_LIMIT) {
            s_screen = SCREEN_ERROR;
            redraw();
            next_req = now + pdMS_TO_TICKS(ERROR_RETRY_MS);
        } else if (s_screen == SCREEN_ERROR && s_fail_count == 0) {
            s_screen = SCREEN_LIST;
            redraw();
            next_req = now;
        }

        // new data arrived
        if (s_dirty) {
            s_dirty = false;
            redraw();
        }

        // scheduled fetch, with a per-state interval
        if ((int32_t)(now - next_req) >= 0) {
            switch (s_screen) {
            case SCREEN_LIST:
                request_fetch(rr);
                rr = (rr + 1) % NUM_SYMBOLS;
                next_req = now + pdMS_TO_TICKS(LIST_REFRESH_MS);
                break;
            case SCREEN_DETAIL:
                request_fetch(s_sel);
                next_req = now + pdMS_TO_TICKS(DETAIL_REFRESH_MS);
                break;
            case SCREEN_ERROR:
                request_fetch(s_sel);
                next_req = now + pdMS_TO_TICKS(ERROR_RETRY_MS);
                break;
            }
        }
    }
}


void ticker_start(QueueHandle_t input_q)
{
    s_lock  = xSemaphoreCreateMutex();
    s_req_q = xQueueCreate(1, sizeof(size_t));
    xTaskCreate(fetch_task,  "fetch",  8192, NULL,    5, NULL);
    xTaskCreate(ticker_task, "ticker", 4096, input_q, 5, NULL);
}























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
            ui_show_quote("AAPL", q.current, q.change);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}