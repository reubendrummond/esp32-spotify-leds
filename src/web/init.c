#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_mac.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "config.h"

#define WIFI_CHANNEL 1
#define MAX_STA_CONN (4)

static const char *TAG = "wifi";
static esp_netif_t *sta_netif = NULL;
static EventGroupHandle_t wifi_event_group;
static esp_netif_ip_info_t ip;

// from https://github.com/espressif/esp-idf/blob/master/examples/wifi/wifi_enterprise/main/wifi_enterprise_main.c
const int CONNECTED_BIT = BIT0;

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
        {
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            esp_wifi_connect();
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);

            // print ip info
            if (esp_netif_get_ip_info(sta_netif, &ip) == ESP_OK)
            {
                ESP_LOGI(TAG, "~~~~~~~~~~~");
                ESP_LOGI(TAG, "IP:" IPSTR, IP2STR(&ip.ip));
                ESP_LOGI(TAG, "MASK:" IPSTR, IP2STR(&ip.netmask));
                ESP_LOGI(TAG, "GW:" IPSTR, IP2STR(&ip.gw));
                ESP_LOGI(TAG, "~~~~~~~~~~~");
            }
        }
    }
}

void wifi_init()
{
    memset(&ip, 0, sizeof(esp_netif_ip_info_t));

    // initialise esp network interface
    ESP_ERROR_CHECK(esp_netif_init());

    // initialise default esp event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // create wifi station in wifi driver
    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    // initialise wifi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // --------- EVENT LOOP CONFIG ---------

    // STA wifi event group
    wifi_event_group = xEventGroupCreate();

    // wifi events
    esp_event_handler_instance_t wifi_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &event_handler,
        NULL,
        &wifi_instance));

    // ip events
    esp_event_handler_instance_t ip_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &event_handler,
        NULL,
        &ip_instance));

    wifi_config_t st_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_OPEN,
            .sae_pwe_h2e = WPA3_SAE_PWE_HASH_TO_ELEMENT,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &st_config));

    // start wifi driver
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "STA init complete");
}