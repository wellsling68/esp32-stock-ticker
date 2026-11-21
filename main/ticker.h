#pragma once

void finnhub_fetch(void *arg);
void ticker_start(QueueHandle_t input_q);

// One row per symbol: name + last price/% (skip the numbers where valid[i] is false),
// with row `selected` highlighted.

void ticker_task(void *arg);
// static void handle_event(input_evt_t e, TickType_t *next_req);
// static size_t next_sym(size_t i, bool cw);
void fetch_task(void *arg);
// static void request_fetch(size_t i);

// static void fetch_symbol(size_t i);
// static void redraw(void);


// static void snapshot(finnhub_quote_t *q, bool *v);