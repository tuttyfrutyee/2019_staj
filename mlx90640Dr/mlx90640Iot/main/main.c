#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "tcpip_adapter.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "esp_timer.h"


#include "wifi_helper.h"
#include "mqtt_helper.h"

#include "tImageCollector.h"

static const char *TAG = "MLX90640IOT_MAIN";

//termal float data
static float termalFloatData[32 * 24];
static char termalImageAsString[768 * 4 + 1]; //since there is 768 pixels and all have 4 digit temperature values( ex: 25.68 °C --> "2568", used 2 digits for precision)
const char* termalImageTopicName = "kuartis/davlumbaz/termalImage"; // "deneme"; 


EventGroupHandle_t mqtt_publish_event_group;
const int PUBLISH_SENT_SUCCESS = BIT0;


esp_mqtt_client_handle_t mqttC; // mmqt client


QueueHandle_t xQueue_termalData;

//image publisher thread handler
TaskHandle_t xHandle_imagePublisher = NULL;
TaskHandle_t xHandle_termalImageCollector = NULL;


esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            vTaskDelay(400 / portTICK_PERIOD_MS);
            mqttStartASync(&mqtt_event_handler_cb);
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);

            //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);                        
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:               
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

void termalImagePublisher(void* args){
    //first check if new data is avaiable
    while(true){

        //printf("waiting mqtt_publish_event_group bits\n");

/*         xEventGroupWaitBits(
            mqtt_publish_event_group,   // The event group being tested.
            PUBLISH_SENT_SUCCESS, // The bits within the event group to wait for.
            pdTRUE,        // should be cleared before returning.
            pdFALSE,    // Don't wait for both bits, either bit will do.
            portMAX_DELAY ); */
        
        //printf("receiving termal data from queue");

        

        xQueueReceive( xQueue_termalData, termalFloatData, portMAX_DELAY );

        //int64_t startTime =  esp_timer_get_time();
        

/*         for(int i = 0; i < 768; i++){
                    sprintf(termalImageAsString + i * 4, "%d", (int)(termalFloatData[i] * 100));
        } */



        int msg_id;


        //msg_id = esp_mqtt_client_publish(mqttC, termalImageTopicName, termalImageAsString, 0, 1, 0); //(char*)mlx90640To, 0, 1, 0);
        msg_id = esp_mqtt_client_publish(mqttC, termalImageTopicName, (char*)termalFloatData, 4 * 768, 1, 0); //(char*)mlx90640To, 0, 1, 0);
        //ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);   
        //printf("time spend in mili seconds receiving data : %llf \n",(esp_timer_get_time() - startTime) / 1000.0 );     

     }
}


void app_main()
{

    ESP_ERROR_CHECK(nvs_flash_init());

    //start wifi syncronously
    wifiStartSync();

    //create queue
    xQueue_termalData = xQueueCreate( 1, 32 * 24 * sizeof(float) );

    //create event for data management
    mqtt_publish_event_group = xEventGroupCreate();
 
    //start mqtt async    
    mqttC = mqttStartASync(&mqtt_event_handler_cb);

    //create and start imagePublisher thread
    xTaskCreate( termalImagePublisher, "imagePublisher", 768 * 4, NULL, NULL, &xHandle_imagePublisher );    

    //start collection termal images
    xTaskCreate(startCollectTImages, "collectTermalImages", 768 * 20, &xQueue_termalData, NULL, &xHandle_termalImageCollector);
    //startCollectTImages(&xQueue_termalData);



}
