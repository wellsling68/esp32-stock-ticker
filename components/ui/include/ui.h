#pragma once
#include "esp_err.h"

// typedef struct {
//     const char *symbol;     // "AAPL"
//     const char *name;       // "Apple Inc."
//     float price, change, change_pct;
//     float open, prev_close, high, low;
// } ui_quote_t;

esp_err_t lvgl_init(void);
void ui_show_quote(const char *symbol, float price, float change_pct);