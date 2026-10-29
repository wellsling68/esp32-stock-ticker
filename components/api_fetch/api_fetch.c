#include "api_fetch.h"
#include "api_secrets.h"

#include <stdio.h>
#include <string.h>
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "cJSON.h"

static const char *TAG = "finnhub";
#define RESP_BUF_SIZE 1024

typedef struct {
    char buf[RESP_BUF_SIZE];
    int len;
} resp_t;

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    resp_t *r = (resp_t *)evt->user_data;
    if (evt->event_id == HTTP_EVENT_ON_DATA && r) {
        int copy = evt->data_len;
        if (r->len + copy >= RESP_BUF_SIZE) copy = RESP_BUF_SIZE - 1 - r->len;
        if (copy > 0) {
            memcpy(r->buf + r->len, evt->data, copy);
            r->len += copy;
            r->buf[r->len] = '\0';
        }
    }
    return ESP_OK;
}

esp_err_t finnhub_get_quote(const char *symbol, finnhub_quote_t *out)
{
    resp_t resp = { .len = 0 };
    char url[160];
    snprintf(url, sizeof(url),
             "https://finnhub.io/api/v1/quote?symbol=%s&token=%s",
             symbol, CONFIG_FINNHUB_API_KEY);

    esp_http_client_config_t cfg = {
        .url = url,
        .event_handler = http_event_handler,
        .user_data = &resp,
        .crt_bundle_attach = esp_crt_bundle_attach, // validates TLS cert
        .timeout_ms = 8000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    esp_err_t err = esp_http_client_perform(client);
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP failed: %s", esp_err_to_name(err));
        return err;
    }
    if (status != 200) {
        ESP_LOGE(TAG, "HTTP status %d", status);
        return ESP_FAIL;
    }

    cJSON *root = cJSON_Parse(resp.buf);
    if (!root) return ESP_FAIL;

    out->current        = (float)cJSON_GetObjectItem(root, "c")->valuedouble;
    out->change         = (float)cJSON_GetObjectItem(root, "d")->valuedouble;
    out->percent_change = (float)cJSON_GetObjectItem(root, "dp")->valuedouble;
    out->high           = (float)cJSON_GetObjectItem(root, "h")->valuedouble;
    out->low            = (float)cJSON_GetObjectItem(root, "l")->valuedouble;
    out->open           = (float)cJSON_GetObjectItem(root, "o")->valuedouble;
    out->prev_close     = (float)cJSON_GetObjectItem(root, "pc")->valuedouble;
    out->timestamp      = (uint32_t)cJSON_GetObjectItem(root, "t")->valuedouble;

    cJSON_Delete(root);
    return ESP_OK;
}