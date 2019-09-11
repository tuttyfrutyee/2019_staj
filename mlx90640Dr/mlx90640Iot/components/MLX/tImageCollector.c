#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"
#include "tImageCollector.h"
#include "esp_log.h"


#define TA_SHIFT 8 
#define WIDTH 32
#define HEIGHT 24

static const char *TAG = "TImageCollector";

static char termalImageAsString[768 * 4 + 1]; //since there is 768 pixels and all have 4 digit temperature values( ex: 25.68 °C --> "2568", used 2 digits for precision)

/**
 * @brief Collects termal images from mlx with driver continuously, should be called with a thread to prevent blocking main thread
 * 
 * @param termalEvent 
 * @param termalImageString 
 * 
 */
esp_err_t startCollectTImages(QueueHandle_t* xQueue_termalData){

    esp_err_t err = initI2C(); 

    if(err!=ESP_OK){
        ESP_LOGE(TAG,"Failed while init i2c"); 
        return err;
    }
    else
    {
            uint16_t *eeMLX90640 = calloc(832,sizeof(uint16_t));
            paramsMLX90640 *mlx90640 = calloc(1,sizeof(paramsMLX90640));
            uint16_t *mlx90640Frame = calloc(834 * 1,sizeof(uint16_t));
            float* mlx90640To = calloc(768,sizeof(float));          
            float emissivity = 0.95; 

            int status;
        // status = MLX90640_SetInterleavedMode (0x33);        
            status = MLX90640_DumpEE (0x33, eeMLX90640);
            status = MLX90640_ExtractParameters(eeMLX90640, mlx90640);

            MLX90640_SetRefreshRate(0x33, 0x04); //Set rate to 16Hz ( 8Hz in fact since one needs two subpages to get an complete frame)
            
            float tr = 0;
            while(true){

                for(int count = 0; count < 2; count++){
                        status = MLX90640_GetFrameData(0x33, mlx90640Frame);
                        tr = MLX90640_GetTa(mlx90640Frame, mlx90640) - TA_SHIFT;
                        MLX90640_CalculateTo(mlx90640Frame, mlx90640, emissivity, tr, mlx90640To);
                }



                MLX90640_BadPixelsCorrection((mlx90640)->brokenPixels, mlx90640To, 1, mlx90640);
                MLX90640_BadPixelsCorrection((mlx90640)->outlierPixels, mlx90640To , 1, mlx90640);

                xQueueOverwrite(*xQueue_termalData , mlx90640To);



/*                 for(int i = 0; i < 768; i++){
                    sprintf(termalImageString + i * 4, "%d", (int)(mlx90640To[i] * 100));
                    //termalImageString[i] = 'a';
                }

                //termalImageString[768 * 4] = '\0';

                //printf("%s\n",termalImageString);
                    

                //ESP_LOG(TAG,"Got an image");    

                xEventGroupSetBits(*termal_event_group, NEW_TDATA_AVAILABLE); */
                

            }

    }

}

