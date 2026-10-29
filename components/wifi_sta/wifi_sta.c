#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#define MAX_RETRIES 10

// Tag shown in front of log messages from this file (e.g. "I (1234) wifi: ...")
static const char *WIFI_TAG = "wifi";

// Handle to the "event group": a set of on/off flags used to signal the
// result of the connection attempt to wifi_connect(), which is waiting on it
static EventGroupHandle_t s_events;

// How many times we've retried connecting after a failure
static int s_retries = 0;

// The two flags (bits) in the event group:
//   CONNECTED_BIT = "we got an IP address, we're online"
//   FAIL_BIT      = "we ran out of retries, giving up"
#define CONNECTED_BIT BIT0
#define FAIL_BIT      BIT1

// Called automatically by the event system whenever Wi-Fi or IP news happens.
//   base = which category of event (WIFI_EVENT or IP_EVENT)
//   id   = which specific event in that category
//   data = extra info about the event (e.g. the IP address)
// (arg is unused here; it's whatever we passed when registering, which was NULL)
static void handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    // Event: the Wi-Fi driver has finished starting up
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        // Now that it's running, tell it to connect to the router
        esp_wifi_connect();

    // Event: the connection failed, or an existing connection was lost
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retries < MAX_RETRIES) {
            // Still have attempts left: count this one and try again
            s_retries++;
            esp_wifi_connect();
        } else {
            // Out of attempts: raise the FAIL flag so wifi_connect() stops waiting
            xEventGroupSetBits(s_events, FAIL_BIT);
        }

    // Event: the router assigned us an IP address (we're fully online)
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        // 'data' is really a pointer to a struct holding the IP info; cast it
        // so we can read the fields
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)data;

        // Print the IP (IPSTR / IP2STR are helper macros for formatting it)
        ESP_LOGI(WIFI_TAG, "Got IP: " IPSTR, IP2STR(&e->ip_info.ip));

        // Reset the retry counter so a future disconnect gets a fresh 5 tries
        s_retries = 0;

        // Raise the CONNECTED flag so wifi_connect() stops waiting
        xEventGroupSetBits(s_events, CONNECTED_BIT);
    }
}

// Sets up Wi-Fi, connects, and waits. Returns true if connected, false if failed.
bool wifi_connect(void)
{
    // --- Step 1: Flash storage (NVS) ---
    // The Wi-Fi driver stores some data in NVS, so it must be initialized first
    // (NVS = "non-volatile storage", a small key/value area in flash)
    esp_err_t ret = nvs_flash_init();

    // If the storage is full or from an older format, erase it and start fresh
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    // ESP_ERROR_CHECK aborts (and reboots) with an error message if ret isn't OK
    ESP_ERROR_CHECK(ret);

    // --- Step 2: Networking groundwork ---
    // Create the set of flags that the handler uses to signal us
    s_events = xEventGroupCreate();

    // Start the TCP/IP networking layer (handles IP addresses, sockets, etc.)
    ESP_ERROR_CHECK(esp_netif_init());

    // Create the system event loop (the thing that delivers events to handlers)
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create a network interface for station mode ("sta" = a device that
    // joins a Wi-Fi network, as opposed to acting as the access point)
    esp_netif_create_default_wifi_sta();

    // --- Step 3: Wi-Fi driver ---
    // Load the default driver settings and initialize the Wi-Fi driver with them
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Tell the event loop: "call handler() for any Wi-Fi event"
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &handler, NULL));
    // ...and "call handler() when we get an IP address"
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &handler, NULL));

    // --- Step 4: Credentials ---
    // Create the config struct, zeroed out so unused fields are blank
    wifi_config_t wc = { 0 };

    // Copy the network name and password into the struct
    // (sizeof makes sure we don't write past the end of the buffer)
    strncpy((char *)wc.sta.ssid, WIFI_SSID, sizeof(wc.sta.ssid));
    strncpy((char *)wc.sta.password, WIFI_PASS, sizeof(wc.sta.password));

    // Minimum security we'll accept: WPA2 or stronger. This stops the ESP32
    // from connecting to a weaker or open network with the same name.
    wc.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    // --- Step 5: Start it up ---
    // Set the mode to station (client)
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Apply the credentials above to the station interface
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));

    // Start the driver. This returns immediately; it triggers the STA_START
    // event, which makes handler() call esp_wifi_connect(). Everything after
    // this happens in the background.
    ESP_ERROR_CHECK(esp_wifi_start());

    // --- Step 6: Wait for the result ---
    // Sleep (no CPU use) until either CONNECTED_BIT or FAIL_BIT is set.
    //   pdFALSE (1st): don't clear the bits after we read them
    //   pdFALSE (2nd): wake when EITHER bit is set (not requiring both)
    //   portMAX_DELAY: wait as long as it takes, no timeout
    EventBits_t bits = xEventGroupWaitBits(s_events, CONNECTED_BIT | FAIL_BIT,
                                           pdFALSE, pdFALSE, portMAX_DELAY);

    // True if the CONNECTED flag is set, false if we got here because of FAIL
    return (bits & CONNECTED_BIT) != 0;
}
