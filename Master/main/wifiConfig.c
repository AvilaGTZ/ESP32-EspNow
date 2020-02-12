#include "wifiConfig.h"
#include "espNow.h"
#include "mqtt.h"

#define MAXIMUM_RETRY  10                               /*Max retries for Wifi connection*/
#define WIFI_SSID "yourSSID"                              /*WIFI SSID*/
#define WIFI_PASS "yourPassword"                          /*SSID Password*/

static EventGroupHandle_t     s_wifi_event_group;       /*Handler to manage WiFi*/
 
const int WIFI_CONNECTED_BIT   = BIT0;           
static const char *TAG         = "CONNECTING TO WIFI: ";
static int s_retry_num         = 0;

/*Wifi event handler-------------------------------------------------------------*/
esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "Connected and assigned ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        break;
        
    case SYSTEM_EVENT_STA_DISCONNECTED:
        {
            if (s_retry_num < MAXIMUM_RETRY) {
                esp_wifi_connect();
                xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                s_retry_num++;
                ESP_LOGI(TAG,"retry to connect to the AP");
            }
            ESP_LOGI(TAG,"connect to the AP fail\n");
            break;
        }
    default:
        break;
    }
    return ESP_OK;
}

void wifi_init_sta()
{
    esp_now_deinit();
    esp_wifi_stop();
    s_wifi_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );

    
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_start() );
    esp_wifi_stop();
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             WIFI_SSID, WIFI_PASS);
}




