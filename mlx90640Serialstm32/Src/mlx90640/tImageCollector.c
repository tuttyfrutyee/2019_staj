
#include "include/MLX90640_I2C_Driver.h"
#include "include/MLX90640_API.h"
#include "include/tImageCollector.h"

#include <stdio.h>


#define TA_SHIFT 8
#define WIDTH 32
#define HEIGHT 24



void getThermalArray(ThermalBag* thermalBag){


            int status;
        // status = MLX90640_SetInterleavedMode (0x33);
            status = MLX90640_DumpEE (0x33, &(thermalBag->eeMLX90640));
            if(status == -1) printf("dumpee failed\n");
            status = MLX90640_ExtractParameters(&(thermalBag->eeMLX90640), &(thermalBag->mlx90640));

            MLX90640_SetRefreshRate(0x33, 0x05); //Set rate to 16Hz ( 8Hz in fact since one needs two subpages to get an complete frame)

            float tr = 0;

			for(int count = 0; count < 2; count++){
					status = MLX90640_GetFrameData(0x33, &(thermalBag->mlx90640Frame));
					tr = MLX90640_GetTa(&(thermalBag->mlx90640Frame), &(thermalBag->mlx90640)) - TA_SHIFT;
					MLX90640_CalculateTo(&(thermalBag->mlx90640Frame), &(thermalBag->mlx90640), thermalBag->emissivity, tr, &(thermalBag->mlx90640To));
			}


			MLX90640_BadPixelsCorrection((thermalBag->mlx90640).brokenPixels, &(thermalBag->mlx90640To), 1, &(thermalBag->mlx90640));
			MLX90640_BadPixelsCorrection((thermalBag->mlx90640).outlierPixels, &(thermalBag->mlx90640To) , 1, &(thermalBag->mlx90640));


			//termal array is ready in thermalBag->mlx90640To

}

