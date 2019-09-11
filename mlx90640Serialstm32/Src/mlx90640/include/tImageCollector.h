
#include "MLX90640_API.h"

typedef struct ThermalBag {

	uint16_t eeMLX90640[832];
	paramsMLX90640 mlx90640;
	uint16_t mlx90640Frame[834];
	float mlx90640To[768];
	float emissivity;

} ThermalBag;

void getThermalArray(ThermalBag* thermalBag);

