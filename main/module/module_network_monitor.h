#pragma once

#include <stdbool.h>
#include "esp_err.h"

esp_err_t module_network_monitor_init(uint32_t repeat_ms);

esp_err_t module_network_monitor_check_sync();

bool module_network_monitor_is_available();
