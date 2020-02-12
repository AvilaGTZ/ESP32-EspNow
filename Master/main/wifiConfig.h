#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <time.h>
#include <sys/time.h>
#include <http_server.h>
#include "esp_http_client.h"


#include "lwip/err.h"
#include "lwip/sys.h"


esp_err_t event_handler(void *ctx, system_event_t *event);
void wifi_init_sta();
void testConnection();