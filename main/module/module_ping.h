#pragma once

#include <stdbool.h>
#include "esp_err.h"

typedef void (*module_ping_callback_func)(void *args);

typedef struct {
    module_ping_callback_func on_ping_success;
    module_ping_callback_func on_ping_timeout;
    module_ping_callback_func on_ping_end;
} module_ping_callback_t;

void module_set_ping_callback(module_ping_callback_t *callback);

esp_err_t module_ping_exec(char target[], uint32_t count, uint32_t interval_ms, uint32_t timeout_ms);

esp_err_t module_ping_exec_once(char target[], uint32_t timeout_ms);

