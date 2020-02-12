#include "esp_log.h"
#include "string.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_now.h"
#include "cJSON.h"

#include "lwip/err.h"
#include "lwip/sys.h"


void startEspNow();
void onReceiveData(const uint8_t *mac, const uint8_t *data, int len);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void sendDataOverEspNow(char *message);
void conectionMode();
void initMessage();
void mainMessage();
void espNowReInit();