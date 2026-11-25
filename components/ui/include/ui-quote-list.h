#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_lvgl_port.h"
#include "api-fetch.h"

void quote_list_create(lv_obj_t *list_screen);
void quote_list_update(const char *const *symbols, const char *const *names,
                       const finnhub_quote_t *q, const bool *valid,
                       size_t count, size_t selected);