#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_event.h"
#include "freertos/event_groups.h"


esp_err_t startCollectTImages(QueueHandle_t* xQueue_termalData);

static const int NEW_TDATA_AVAILABLE = BIT0;