#include "module_ping.h"
#include "ping/ping_sock.h"
#include "argtable3/argtable3.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static char *TAG = "module_ping";

static module_ping_callback_t *s_callback = NULL;

static void cmd_ping_on_ping_success(esp_ping_handle_t hdl, void *args) {
    char * target = args;
    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    ESP_LOGI(TAG, "%lu bytes from %s(%s) icmp_seq=%d ttl=%d time=%lu ms",
                 recv_len, target, ipaddr_ntoa((ip_addr_t *) &target_addr), seqno, ttl, elapsed_time);

    if (s_callback) {
        s_callback->on_ping_success(args);
    }
}

static void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *args) {
    char * target = args;
    uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    ESP_LOGW(TAG, "From %s(%s) icmp_seq=%d timeout", target, ipaddr_ntoa((ip_addr_t *) &target_addr), seqno);

    if (s_callback) {
        s_callback->on_ping_timeout(args);
    }
}

static void cmd_ping_on_ping_end(esp_ping_handle_t hdl, void *args) {
    esp_ping_delete_session(hdl);

    if (s_callback) {
        s_callback->on_ping_end(args);
    }
}

esp_err_t module_ping_exec(char target[], uint32_t count, uint32_t interval_ms, uint32_t timeout_ms) {
    esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();
    config.count = count;
    config.interval_ms = interval_ms;
    config.timeout_ms = timeout_ms;

    // parse IP address
    struct sockaddr_in6 sock_addr6;
    ip_addr_t target_addr;
    memset(&target_addr, 0, sizeof(target_addr));

    if (inet_pton(AF_INET6, target, &sock_addr6.sin6_addr) == 1) {
        /* convert ip6 string to ip6 address */
        ipaddr_aton(target, &target_addr);
    } else {
        struct addrinfo hint;
        struct addrinfo *res = NULL;
        memset(&hint, 0, sizeof(hint));
        /* convert ip4 string or hostname to ip4 or ip6 address */
        if (getaddrinfo(target, NULL, &hint, &res) != 0) {
            ESP_LOGE(TAG, "ping: unknown host %s\n", target);
            return ESP_FAIL;
        }
        if (res->ai_family == AF_INET) {
            struct in_addr addr4 = ((struct sockaddr_in *) (res->ai_addr))->sin_addr;
            inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
        } else {
            struct in6_addr addr6 = ((struct sockaddr_in6 *) (res->ai_addr))->sin6_addr;
            inet6_addr_to_ip6addr(ip_2_ip6(&target_addr), &addr6);
        }
        freeaddrinfo(res);
    }
    config.target_addr = target_addr;

    /* set callback functions */
    esp_ping_callbacks_t cbs = {
            .cb_args = target,
            .on_ping_success = cmd_ping_on_ping_success,
            .on_ping_timeout = cmd_ping_on_ping_timeout,
            .on_ping_end = cmd_ping_on_ping_end
    };
    esp_ping_handle_t ping;
    esp_ping_new_session(&config, &cbs, &ping);
    esp_ping_start(ping);

    return ESP_OK;
}

esp_err_t module_ping_exec_once(char target[], uint32_t timeout_ms) {
    return module_ping_exec(target, 1, 1000, timeout_ms);
}

void module_set_ping_callback(module_ping_callback_t *callback) {
    s_callback = callback;
}