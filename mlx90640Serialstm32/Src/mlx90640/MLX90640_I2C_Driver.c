/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */



#include "include/MLX90640_I2C_Driver.h"
#include <stdio.h>

extern I2C_HandleTypeDef hi2c1;

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
	HAL_StatusTypeDef status;
	uint8_t i2cData[nMemAddressRead * 2];
	status = HAL_I2C_Mem_Read(&hi2c1, slaveAddr << 1, startAddress, I2C_MEMADD_SIZE_16BIT, i2cData, nMemAddressRead * 2  , 10);
	if(status != HAL_OK){
		printf("i2c read error : %d\n",status);
		return -1;
	}

	for(int cnt=0; cnt < nMemAddressRead; cnt++)
	{
		uint16_t i = cnt << 1;
		uint16_t tmp=0;
		tmp = i2cData[i];
		tmp = tmp<<8;
		tmp |= (uint16_t)i2cData[i+1];
		data[cnt] = tmp;
	}

	return 0;

}


int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
	HAL_StatusTypeDef status;
	uint8_t stack[2] = {0,0};
	stack[0] = (uint8_t)(data << 8);
	stack[1] = (uint8_t)(data & 0x00FF);

	status = HAL_I2C_Mem_Write(&hi2c1, slaveAddr << 1, writeAddress, I2C_MEMADD_SIZE_16BIT, stack, 2, 10);
	if(status != HAL_OK){
		printf("i2c write error :%d\n",status);
		return -1;
	}

	return 0;
}

