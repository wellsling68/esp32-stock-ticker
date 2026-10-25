#pragma once
#include <stdbool.h>

// Connects to Wi-Fi and blocks until connected or failed.
// Returns true if connected.
bool wifi_connect(void);