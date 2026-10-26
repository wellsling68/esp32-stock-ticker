#pragma once
#include <stdint.h>
#include "esp_err.h"

typedef struct {
    float current;        // c
    float change;         // d
    float percent_change; // dp
    float high;           // h
    float low;            // l
    float open;           // o
    float prev_close;     // pc
    uint32_t timestamp;   // t
} finnhub_quote_t;

esp_err_t finnhub_get_quote(const char *symbol, finnhub_quote_t *out);