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
#ifndef _MLX90640_I2C_Driver_H_
#define _MLX90640_I2C_Driver_H_

#include <stdint.h>
#include "esp_err.h"

    #define I2C_MASTER_TX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */
    #define I2C_MASTER_RX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */
    #define WRITE_BIT 0                                 /*!< I2C master write */
    #define READ_BIT 1                                  /*!< I2C master read */
    #define ACK_CHECK_EN 0x1                            /*!< I2C master will check ack from slave*/
    #define I2C_ACK_VAL 0x0                             /*!< I2C ack value */
    #define I2C_NACK_VAL 0x1                            /*!< I2C nack value */

    //I2C CLASS DEFAULTS
    #define I2C_MASTER_SCL_IO GPIO_NUM_22               /*!< gpio number for I2C master clock for esp32 devkitvc-1 */
    #define I2C_MASTER_SDA_IO GPIO_NUM_21               /*!< gpio number for I2C master data for esp32 devkitvc-1  */
    #define I2C_MASTER_NUM 1                            /*!< I2C port number for master dev */
    #define I2C_MASTER_FREQ_HZ 900000                   /*!< I2C master clock frequency */





    //void MLX90640_I2CFreqSet(int freq); not used

    #ifdef __cplusplus
    extern "C" {
    #endif

        void MLX90640_I2CInit(void);
        int MLX90640_I2CRead(uint8_t slaveAddr,uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data);
        int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data);
        esp_err_t initI2C(void);

    #ifdef __cplusplus
    }
    #endif

#endif
