#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum { INPUT_EVT_CW, INPUT_EVT_CCW, INPUT_EVT_PRESS } input_evt_t;

// Starts the input task; returns the queue that delivers input_evt_t events.
QueueHandle_t input_init(void);