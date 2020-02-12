
#include "espNow.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"

#define SLEEP_TIME 60000000
/*Master mac address MAC: 24:0a:c4:81:9f:5c*/
uint8_t broadcastAddress[]    = {0x24, 0x0a,0xc4,0x81,0x9f,0x5c};      
char *TAGEspNow               = "ESPNOW MODE: ";
char *recievedData;
char *welcomeMessage = "NONE";
int  state = 1;
char *compareMessage = "welcome";

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  ESP_LOGI(TAGEspNow,"Last Packet Sent to: "); ESP_LOGI(TAGEspNow,"MAC: %s",macStr);
  ESP_LOGI(TAGEspNow,"Last Packet Send Status: "); 
}


void onReceiveData(const uint8_t *mac, const uint8_t *data, int len) {
 
  ESP_LOGI(TAGEspNow, "Received from MAC:");
  for (int i = 0; i < 6; i++) {
 
    printf("%02X", mac[i]);
    if (i < 5)printf(":");
  }
  recievedData  = (char*)data;
  ESP_LOGI(TAGEspNow, "Data recieved %s",recievedData);
  cJSON *parsedData = cJSON_Parse(recievedData);

  if(parsedData != NULL){
      char *welcomeMessage = cJSON_GetObjectItem(parsedData,"message")->valuestring;
      ESP_LOGI(TAGEspNow, "Received message %s:",welcomeMessage);
      if(*welcomeMessage == *compareMessage){state = 3;}
  }else{
      ESP_LOGI(TAGEspNow, "No data found");
  }

}


void startEspNow(){

    esp_reset_reason_t reason = esp_reset_reason();
    ESP_LOGI(TAGEspNow, "Reset reason %d",reason);
    if (reason == 8) {
        esp_restart();
    }
    if (reason == 3) {
        ESP_LOGI(TAGEspNow, "Go to state 3");
        espNowReInit();
        state = 3;
    }

    while(1){
        if(state == 1){espNowReInit();}
        if(state == 2){initMessage(); vTaskDelay(5000 / portTICK_PERIOD_MS);}
        if(state == 3){
            mainMessage(); 
            esp_now_deinit();
            esp_wifi_stop();
            ESP_LOGI(TAGEspNow, "Going to deepsleep by 1 min");
            esp_sleep_enable_timer_wakeup(SLEEP_TIME);
            esp_deep_sleep_start();
        }
    }

}

void sendDataOverEspNow(char *message){
    
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)message, 250);

    if (result == ESP_OK) {
        ESP_LOGI(TAGEspNow, "Data sent successfully");
    }
    else {
        ESP_LOGI(TAGEspNow, "Error sending data");
    }
}

/*Hello message, can be changed to test the MQTT on Master side---------------------------*/
void initMessage(){
    /*Build the JSON message--------------------------------------------------------------*/
    /*JSON Format variables to create messages---------------------*/
    char  *jsonToString;
    cJSON *mainMessage       = cJSON_CreateObject();
    cJSON *welcomeMessage    = NULL;  

    welcomeMessage = cJSON_CreateString("hello");                   //Original message to start 
    //welcomeMessage = cJSON_CreateString("errorMessage");          //To test the case where master Post MQTT
	cJSON_AddItemToObject(mainMessage,"message",welcomeMessage);
    jsonToString = cJSON_Print(mainMessage); 
    ESP_LOGI(TAGEspNow, "Data to sent:%s ",jsonToString);
    sendDataOverEspNow(jsonToString);

}

/*Message with the type, data and mac information in JSON Format--------------------------*/
void mainMessage(){

    /*Build the JSON message--------------------------------------------------------------*/
    /*JSON Format variables to create messages---------------------*/
    char  *jsonToString;
    int   max                = 4000;
    int   min                = 1;
    int   randData           = rand() % (max - min + 1) + min;
    char  randNum[20];
    sprintf(randNum, "%d", randData);
    cJSON *mainMessage       = cJSON_CreateObject();
    cJSON *type              = NULL; 
    cJSON *data              = NULL;
    cJSON *deviceId          = NULL; 

    type = cJSON_CreateString("sensor"); 
	cJSON_AddItemToObject(mainMessage,"type",type);

    data = cJSON_CreateString((char*)randNum); 
	cJSON_AddItemToObject(mainMessage,"data",data);

    deviceId = cJSON_CreateString("24:0a:c4:a6:12:50");
	cJSON_AddItemToObject(mainMessage,"device",deviceId);

    jsonToString = cJSON_Print(mainMessage); 
    ESP_LOGI(TAGEspNow, "Data to sent:%s ",jsonToString);

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)jsonToString, 250);
    
    if (result == ESP_OK) {
        ESP_LOGI(TAGEspNow, "Data sent successfully");
    }
    else {
        ESP_LOGI(TAGEspNow, "Error sending data");
    }
}

void espNowReInit(){

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_APSTA) );
    ESP_ERROR_CHECK( esp_wifi_start());
    ESP_ERROR_CHECK( esp_wifi_set_channel(6, 0) );
    if (esp_now_init() != ESP_OK) {
        ESP_LOGI(TAGEspNow, "Error at Espnow init");
        return;
    }
    else{
        ESP_LOGI(TAGEspNow, "Espnow init OK");
    }

    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK){
    ESP_LOGI(TAGEspNow, "Error adding peer");
    return;
    }
    else{
        ESP_LOGI(TAGEspNow, "Peer info added");
    }

    /*Call back function to recieve data-----------------------------------------------*/
    esp_now_register_recv_cb(onReceiveData);
    esp_now_register_send_cb(OnDataSent);

    state = 2;

}