#pragma once

#include <stdbool.h>
#include "esp_err.h"

typedef struct {
    uint8_t ssid[32];
    uint8_t password[64];
} module_wifi_sta_config_t;

esp_err_t module_wifi_sta_init(module_wifi_sta_config_t *conf);

bool module_wifi_sta_is_connected(void);