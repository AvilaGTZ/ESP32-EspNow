
#include "espNow.h"
#include "wifiConfig.h"
#include "mqtt.h"

uint8_t broadcastAddress[]    = {0x24, 0x0A,0xC4,0xA6,0x12,0x50};   /*Mac address of the sender device */
char *TAGEspNow               = "ESPNOW MODE: ";
char *testData;
char *recievedData;
int   state                   = 1;       //Initial state        
char *compareMessage          = "hello"; //Compare string for incoming messages
        

esp_now_peer_info_t peerInfo;

/*Function to set new state------------------------------------------------------------------*/
void setState(int sta){
    state = sta;
}

/*Callback function when ESPNow send a message-----------------------------------------------*/
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  ESP_LOGI(TAGEspNow,"Last Packet Sent to: "); ESP_LOGI(TAGEspNow,"MAC: %s",macStr);
  ESP_LOGI(TAGEspNow,"Last Packet Send Status: "); 
}

/*Call back function to recieve data over ESPNow--------------------------------------------*/
void onReceiveData(const uint8_t *mac, const uint8_t *data, int len) {
 
  ESP_LOGI(TAGEspNow, "Received from MAC:");
  for (int i = 0; i < 6; i++) {
 
    printf("%02X", mac[i]);
    if (i < 5)printf(":");
  }

  /*Convert uint8_t data in to char--------------------------------------------------------*/
  recievedData  = (char*)data;
  ESP_LOGI(TAGEspNow, "Data recieved %s",recievedData);
  cJSON *parsedData = cJSON_Parse(recievedData);   //Parse JSON data from incoming message
  cJSON *messageKey = NULL;                        //JSON Object to search "Message" KEY
  cJSON *typeKey = NULL;                           //JSON Object to search "type" KEY

  /*If our incoming message is really a JSON message--------------------------------------*/
  if(parsedData != NULL){

      messageKey = cJSON_GetObjectItem(parsedData,"message"); //Search key "message"
      typeKey    = cJSON_GetObjectItem(parsedData,"type");    //Search key "type"

      /*KEY "message" was found*/
      if(messageKey != NULL){
          
          char *helloMessage = cJSON_GetObjectItem(parsedData,"message")->valuestring;
          ESP_LOGI(TAGEspNow, "Received message %s:",helloMessage);

          /*Message is "hello"*/
          if(*helloMessage == *compareMessage){
              ESP_LOGI(TAGEspNow, "Received message match, will send Welcome message");
              state = 2;
            }
            /*Message is different from "hello (Time to send information over MQTT)*/
          else{
              ESP_LOGI(TAGEspNow, "Received message not match, will send info to MQTT channel");
              state = 3;
              wifi_init_sta();
              initMQTT(recievedData);
              
          }
      }
      /*KEY "type" was found*/
      if(typeKey != NULL){
          char *typeMessage = cJSON_GetObjectItem(parsedData,"type")->valuestring;
          ESP_LOGI(TAGEspNow, "Received message %s:",typeMessage);
      }
       vTaskDelay(3000 / portTICK_PERIOD_MS);

  }else{
      ESP_LOGI(TAGEspNow, "No data found");
  }

}

void startEspNow(){

    /*Init ESPNow protocol ---------------------------------------------------------------------*/
    if (esp_now_init() != ESP_OK) {
        ESP_LOGI(TAGEspNow, "Error at Espnow init");
        return;
    }
    else{
        ESP_LOGI(TAGEspNow, "Espnow init OK");
    }

    /*Add the mac address of our ESP32 Slave---------------------------------------------------*/
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK){
    ESP_LOGI(TAGEspNow, "Error adding peer");
    return;
    }
    else{
        ESP_LOGI(TAGEspNow, "Adding peer OK");
    }

    /*Call back function to recieve and send data-----------------------------------------------*/
    esp_now_register_recv_cb(onReceiveData);
    esp_now_register_send_cb(OnDataSent);

    while(1){
        /*State 1 is where ESP32 Master is waiting for incoming messages from peer devices*/
        if(state == 1){
            ESP_LOGI(TAGEspNow, "Waiting for messages");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        /*State 2 send the acknolegde to the peer device so can send the new information*/
        /*Send the acknolegde 5 times to be sure that will be recieved*/
        if(state == 2){
            for(int i = 0; i<=5; i++){
                ESP_LOGI(TAGEspNow, "Sending acknolegde");
                initMessage(); 
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            state = 1;
        }

    }

}

/*Function to send data over ESPNow, recieve the message that want to be send --------------*/
void sendDataOverEspNow(char *message){
    
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)message, 250);

    if (result == ESP_OK) {
        ESP_LOGI(TAGEspNow, "Data sent successfully");
    }
    else {
        ESP_LOGI(TAGEspNow, "Error sending data");
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
}

/*Initial response message for the slaves devices "Welcome"-------------------------------*/
void initMessage(){
    /*Build the JSON message---------------------------------------*/
    /*JSON Format variables to create messages---------------------*/
    char  *jsonToString;
    cJSON *mainMessage       = cJSON_CreateObject();
    cJSON *welcomeMessage    = NULL;  

    welcomeMessage = cJSON_CreateString("welcome"); 
	cJSON_AddItemToObject(mainMessage,"message",welcomeMessage);
    jsonToString = cJSON_Print(mainMessage); 
    ESP_LOGI(TAGEspNow, "Data to sent:%s ",jsonToString);
    sendDataOverEspNow(jsonToString);

}


