
static const char* brokerUrl = "mqtt://192.168.1.143";


esp_mqtt_client_handle_t mqttStartASync(esp_err_t (*mqtt_event_handler_cb_)(esp_mqtt_event_handle_t event));