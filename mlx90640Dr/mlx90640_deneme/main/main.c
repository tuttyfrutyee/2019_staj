#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"

 #include "esp_log.h"


#define TA_SHIFT 8 
#define WIDTH 32
#define HEIGHT 24

static char * TAG = "MAIN";

/* float calculateMeanOfAnArrayWithoutZeros(float* array,int length){
   float totalsum = 0.0;
   int nonZeroCount = 0;
   for(int i = 0; i < length; i++){
      if(array[i] != 0){
         totalsum += array[i];
         nonZeroCount ++;
      }
         
   }
   return totalsum / nonZeroCount;
} */

/* void removeBiasFromArrayZerosNIncluded(float*array,int length,float bias){
   for(int i = 0; i < length; i++){
      if(array[i] != 0){
         array[i] -= bias;
      }
   }
}
 */
//not used, it turns out that mlx api it self doest it already for both modes(chess mode and interleaved mode)
float* mergeTwoFrameDatas(float*frameData1, float*frameData2){
   float* mergedFrame = calloc(768,sizeof(float));

   float meanFrameData1 = calculateMeanOfAnArrayWithoutZeros(frameData1, WIDTH * HEIGHT);
   float meanFrameData2 = calculateMeanOfAnArrayWithoutZeros(frameData2, WIDTH * HEIGHT);

   printf("meanFrameData1 = %f/n",meanFrameData1);
   printf("meanFrameData2 = %f/n",meanFrameData2);

   removeBiasFromArrayZerosNIncluded(frameData2, WIDTH * HEIGHT, meanFrameData2 - meanFrameData1);

   float*frameData_pN0, *frameData_pN1; // pointers to frameDatas of pageNumber 0 and 1 respectively
  // printf("frameData1[0] = %f")
   if(frameData1[0] > 0.001){
      frameData_pN0 = frameData1;
      frameData_pN1 = frameData2;
   }else{
      frameData_pN0 = frameData2;
      frameData_pN1 = frameData1;
   }

   for(int row = 0; row < HEIGHT; row++){
      for(int col = 0; col < WIDTH; col++){
         if(row%2 == 0){
            if(col%2 == 0)
               //corresponds to pageNumber 0
               mergedFrame[row*WIDTH + col] = frameData_pN0[row*WIDTH + col];
            else
               //corresponds to pageNumber 1
               mergedFrame[row*WIDTH + col] = frameData_pN1[row*WIDTH + col];
         }else{
            if(col%2 == 1)
               //corresponds to pageNumber 0
               mergedFrame[row*WIDTH + col] = frameData_pN0[row*WIDTH + col];
            else
               //corresponds to pageNumber 1
               mergedFrame[row*WIDTH + col] = frameData_pN1[row*WIDTH + col];            
         }

      }
   }

   return mergedFrame;
}


void app_main(void)
{


   esp_err_t err = initI2C();
   if(err!=ESP_OK)
    printf("error occured\n");
   else
   {
        uint16_t *eeMLX90640 = calloc(832,sizeof(uint16_t));
        paramsMLX90640 *mlx90640 = calloc(1,sizeof(paramsMLX90640));
        uint16_t *mlx90640Frame = calloc(834 * 1,sizeof(uint16_t));
        float* mlx90640To = calloc(768,sizeof(float));  
        int * mlx90640To_int = calloc(768,sizeof(int));
        float* frame1,*frame2;
        static float mlx90640Image[768];
        float emissivity = 0.90; 

        int status;
       // status = MLX90640_SetInterleavedMode (0x33);        
        status = MLX90640_DumpEE (0x33, eeMLX90640);
        status = MLX90640_ExtractParameters(eeMLX90640, mlx90640);

        MLX90640_SetRefreshRate(0x33, 0x05); //Set rate to 16Hz ( 8Hz in fact since one needs two subpages to get an complete frame)
        
        float tr = 0;
        while(true){

               for(int count = 0; count < 2; count++){
                     status = MLX90640_GetFrameData(0x33, mlx90640Frame);
                     tr = MLX90640_GetTa(mlx90640Frame, mlx90640) - TA_SHIFT;
                     MLX90640_CalculateTo(mlx90640Frame, mlx90640, emissivity, tr, mlx90640To);
               }



               MLX90640_BadPixelsCorrection((mlx90640)->brokenPixels, mlx90640To, 1, mlx90640);
               MLX90640_BadPixelsCorrection((mlx90640)->outlierPixels, mlx90640To , 1, mlx90640);



               for(int i = 0; i < 768; i++)
                  mlx90640To_int[i] = (int)(mlx90640To[i] * 100);
               
               
               printf("logging image data\n");

               //esp_log_buffer_hex("imageBuffer",mlx90640To_int,768);
               
               //printf("\n");

                for(int row = 0; row < HEIGHT; row++){
                   printf("imageBuffer: ");
                  for(int col = 0; col < WIDTH; col++)
                        printf("%d ",mlx90640To_int[row * WIDTH + col]); 
                  printf("\n");
                  
               }
               printf("\n"); 

        }

  
            

/*         int curRR;
        curRR = MLX90640_GetRefreshRate (0x33);
        ESP_LOGW(TAG,"currFreshrate = %d\n",curRR);

        float vdd;
        vdd = MLX90640_GetVdd( mlx90640Frame, mlx90640); //vdd = 3.3
        ESP_LOGW(TAG,"Vdd = %f\n",vdd); */
   }
   
    
}
