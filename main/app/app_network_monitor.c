#include "app_network_monitor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "module_ping.h"
#include "esp_log.h"

static bool s_network_available = false;
static TimerHandle_t s_timer_handle = NULL;
static QueueHandle_t s_queue_handle = NULL;
static EventGroupHandle_t s_event_group_handle = NULL;

#define NETWORK_AVAILABLE_BIT       BIT0
#define NETWORK_UNAVAILABLE_BIT     BIT1

static char *TAG = "app_network_monitor";

void ping_network_task(void *pvParam) {
    int cmd = 0;
    while (xQueueReceive(s_queue_handle, &cmd, portMAX_DELAY) == pdPASS) {
        if (cmd == 1) {
            module_ping_exec_once("www.bing.com", 2000);
        }
    }
}

static void timer_callback(TimerHandle_t handle) {
    int cmd = 1;
    if (xQueueSend(s_queue_handle, &cmd, pdMS_TO_TICKS(10)) != pdPASS) {
        ESP_LOGW(TAG, "send cmd %d failed", cmd);
    }
}

static void ping_success(void *args) {
    xEventGroupSetBits(s_event_group_handle, NETWORK_AVAILABLE_BIT);
}

static void ping_timeout(void *args) {
    xEventGroupSetBits(s_event_group_handle, NETWORK_UNAVAILABLE_BIT);
}

static void ping_end(void *args) {
}

esp_err_t app_network_monitor_init() {
    static module_ping_callback_t ping_callback;
    ping_callback.on_ping_success = ping_success;
    ping_callback.on_ping_timeout = ping_timeout;
    ping_callback.on_ping_end = ping_end;
    module_set_ping_callback(&ping_callback);

    s_timer_handle = xTimerCreate("network_monitor_timer",
                                  pdMS_TO_TICKS(1000 * 60),
                                  pdTRUE,
                                  NULL,
                                  timer_callback);
    xTimerStart(s_timer_handle, 0);

    s_queue_handle = xQueueCreate(2, sizeof(int));
    xTaskCreate(ping_network_task, "ping_network_task", 2048,
                NULL, 1, NULL);

    s_event_group_handle = xEventGroupCreate();
    return ESP_OK;
}

esp_err_t app_network_monitor_check_sync() {
    int cmd = 1;
    if (xQueueSend(s_queue_handle, &cmd, pdMS_TO_TICKS(10)) != pdPASS) {
        ESP_LOGW(TAG, "send cmd %d failed", cmd);
        return ESP_FAIL;
    }
    EventBits_t bits = xEventGroupWaitBits(s_event_group_handle,
                                           NETWORK_AVAILABLE_BIT | NETWORK_UNAVAILABLE_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & NETWORK_AVAILABLE_BIT) {
        s_network_available = true;
        return ESP_OK;
    } else if (bits & NETWORK_UNAVAILABLE_BIT) {
        s_network_available = false;
        return ESP_FAIL;
    } else {
        s_network_available = false;
        return ESP_FAIL;
    }
}
