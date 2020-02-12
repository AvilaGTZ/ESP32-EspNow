#include "mqtt.h"
#include "wifiConfig.h"
#include "espNow.h"

/*Will try to send the mesasge 5 times, if not, return to ESPNOW Mode*/
#define MAX_ATTEPMTS 5
#define MQTT_SERVER "mqtt://ioticos.org"
#define MQTT_PASS   "B0x3qemgkxDzRTW"
#define MQTT_USER   "6xCBRxY2bgwp5pk"
#define MQTT_PORT   1883

#define TAGMQTT "MQTT: "
esp_mqtt_client_handle_t client;

static const char *TAG = "MQTTMode";
int tries              = 0;
char *recievedDataMQTT;

/*MQTT Handler-------------------------------------------------------------------------------------------*/
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_publish(client, "/0JnzA0efByHmFgS/output", recievedDataMQTT, 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/0JnzA0efByHmFgS/", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/0JnzA0efByHmFgS/", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            /*Send our message was success, time to return to ESPNOW mode----------------------------------*/
            ESP_LOGI(TAG, "Success sendindg message, will return to ESPNOW Mode...");
            vTaskDelay(3000 / portTICK_PERIOD_MS);
            esp_restart();
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/0JnzA0efByHmFgS/output", recievedDataMQTT, 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            tries++;
            if(tries >= MAX_ATTEPMTS){
                esp_restart();
            }
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}


/*MQTT Init-------------------------------------------------------------------*/
void initMQTT(char *data){
    recievedDataMQTT = data;
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = MQTT_SERVER,               
        .port = MQTT_PORT,
        .username = MQTT_USER,
        .password = MQTT_PASS,
        .event_handle = mqtt_event_handler
       
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
    ESP_LOGI(TAGMQTT, "MQTT Init OK");
}

void sendInformationMQTT(char *data){

    esp_mqtt_client_publish(client, "/0JnzA0efByHmFgS/output", "data_3", 0, 1, 0);

}