#include "esp_now.h"
#include "esp_log.h"
#include "string.h"
#include "esp_system.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"




void startEspNow();
void sendDataOverEspNow(char *message);
void onReceiveData(const uint8_t *mac, const uint8_t *data, int len);
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void mainMessage();
void initMessage();
void setState(int sta);
void wifi_init_espnow();