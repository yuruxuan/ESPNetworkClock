#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t app_network_monitor_init();

esp_err_t app_network_monitor_check_sync();

bool app_network_monitor_is_available();
