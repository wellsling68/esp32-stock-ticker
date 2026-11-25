#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"
#include "esp_log.h"
#include "api-fetch.h"

// typedef struct {
//     const char *symbol;     // "AAPL"
//     const char *name;       // "Apple Inc."
//     float price, change, change_pct;
//     float open, prev_close, high, low;
// } ui_quote_t;

// typedef struct {
//     const char *symbol;
//     const char *name;
//     float price, change, change_pct;
//     float prev_close, open, high, low;
//     bool valid;
// } list_quote_t;

// void ui_show_list(const ui_quote_t *items, size_t count, size_t selected);
void ui_show_list(const char *const *symbols, const char *const *names,
                  const finnhub_quote_t *q, const bool *valid,
                  size_t count, size_t selected);


esp_err_t lvgl_init(void);

void ui_show_quote(const char *symbol, float price, float change_pct);

// void ui_show_list(const char *const *symbols, const float *price, const float *pct,
//                   const bool *valid, size_t count, size_t selected);
void ui_show_error(const char *msg);