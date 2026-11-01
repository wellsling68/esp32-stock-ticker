#include "input.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define PIN_A    GPIO_NUM_32   // change to your pins
#define PIN_B    GPIO_NUM_33
#define PIN_BTN  GPIO_NUM_25

#define POLL_MS       1
#define BTN_DEBOUNCE  20   // consecutive samples (ms) before accepting a change

static const char *TAG = "input";
static QueueHandle_t s_queue;

// index = (prev_state << 2) | curr_state, state = (A << 1) | B
// invalid/bounce transitions map to 0
static const int8_t k_table[16] = {
     0, +1, -1,  0,
    -1,  0,  0, +1,
    +1,  0,  0, -1,
     0, -1, +1,  0,
};

static void input_task(void *arg)
{
    uint8_t prev = (gpio_get_level(PIN_A) << 1) | gpio_get_level(PIN_B);
    int acc = 0;

    int btn_stable = 1;      // 1 = released (pull-up)
    int btn_count = 0;

    while (1) {
        uint8_t curr = (gpio_get_level(PIN_A) << 1) | gpio_get_level(PIN_B);
        if (curr != prev) {
            acc += k_table[(prev << 2) | curr];
            prev = curr;
            if (acc >= 4) {
                input_evt_t e = INPUT_EVT_CW;
                xQueueSend(s_queue, &e, 0);
                acc = 0;
            } else if (acc <= -4) {
                input_evt_t e = INPUT_EVT_CCW;
                xQueueSend(s_queue, &e, 0);
                acc = 0;
            }
        }
        if (curr == 0x3) acc = 0;   // back at rest: drop any partial/aborted turn

        int btn = gpio_get_level(PIN_BTN);
        if (btn != btn_stable) {
            if (++btn_count >= BTN_DEBOUNCE) {
                btn_stable = btn;
                btn_count = 0;
                if (btn_stable == 0) {
                    input_evt_t e = INPUT_EVT_PRESS;
                    xQueueSend(s_queue, &e, 0);
                }
            }
        } else {
            btn_count = 0;
        }

        TickType_t d = pdMS_TO_TICKS(POLL_MS);
        vTaskDelay(d ? d : 1);
    }
}

QueueHandle_t input_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << PIN_A) | (1ULL << PIN_B) | (1ULL << PIN_BTN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));

    s_queue = xQueueCreate(16, sizeof(input_evt_t));
    xTaskCreate(input_task, "input", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "ready (A=%d B=%d BTN=%d)", PIN_A, PIN_B, PIN_BTN);
    return s_queue;
}