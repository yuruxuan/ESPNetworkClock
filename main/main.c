#include <stdio.h>
#include "app_network_monitor.h"
#include "module_nvs.h"
#include "module_wifi_sta.h"
#include "module_sntp.h"
#include "esp_log.h"

static char *TAG = "app_main";

void app_main(void) {
    ESP_ERROR_CHECK(module_nvs_flash_init());

    module_wifi_sta_config_t wifi_sta_config = {
            .ssid = "",
            .password = ""
    };
    ESP_ERROR_CHECK(module_wifi_sta_init(&wifi_sta_config));

    ESP_ERROR_CHECK(app_network_monitor_init());
    ESP_ERROR_CHECK(app_network_monitor_check_sync());

    ESP_ERROR_CHECK(module_sntp_init());

    ESP_LOGI(TAG, "Init finished!");
}
