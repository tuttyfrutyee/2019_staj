#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "esp_event.h"
#include "tcpip_adapter.h"
#include "mqtt_client.h"
#include "mqtt_helper.h"




static const char *TAG = "MQTT HELPER";

static esp_err_t (*mqtt_event_handler_cb)(esp_mqtt_event_handle_t event) = NULL;


static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    (*mqtt_event_handler_cb)(event_data);
}

static esp_mqtt_client_handle_t mqtt_app_start(void)
{
    
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = brokerUrl,
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
    return client;
}

esp_mqtt_client_handle_t mqttStartASync(esp_err_t (*mqtt_event_handler_cb_)(esp_mqtt_event_handle_t event)){ //accepts a callback from main function to handlee mqtt events
    mqtt_event_handler_cb = mqtt_event_handler_cb_;
    return mqtt_app_start();
}