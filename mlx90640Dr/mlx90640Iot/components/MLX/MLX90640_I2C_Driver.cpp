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



#include "driver/i2c.h"
#include "esp_log.h"
#include "MLX90640_I2C_Driver.h"


char * TAG = "I2C";



esp_err_t initI2C(void){
    i2c_config_t conf;
    int i2c_master_port = I2C_MASTER_NUM;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    return  i2c_driver_install(i2c_master_port, conf.mode,
                            I2C_MASTER_RX_BUF_DISABLE,
                            I2C_MASTER_TX_BUF_DISABLE, 0);     
}

void MLX90640_I2CInit()
{   
    //mlx wants sda line high before starting to trasmission  
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);  
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
                              
    uint16_t cnt = 0; //counter
    uint8_t stack[2] = {0,0}; //seperation for 16bit register to 8bits
    uint8_t i2cData[1664] = {0}; //preallocation
    uint16_t nMemAddressReadx2 = nMemAddressRead << 1;
    //printf("allocated i2cData successfully \n");
    
    stack[0] = startAddress >> 8;
    stack[1] = startAddress & 0x00FF;
    
    //writing the 16bit targetRegisterAddress
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    //i2c_master_stop(cmd);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( slaveAddr << 1 ) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, stack[0], true);
    i2c_master_write_byte(cmd, stack[1], true);
    //i2c_master_stop(cmd);
        
    //starting to read from target address
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( slaveAddr << 1 ) | I2C_MASTER_READ, true);

    nMemAddressReadx2 = nMemAddressRead *2; // since the data is 16 bit and i2c is reading 8 bit, hence the double of readnumber
    if (nMemAddressReadx2 > 1) i2c_master_read(cmd, i2cData, nMemAddressReadx2-1, I2C_ACK_VAL);
    i2c_master_read_byte(cmd, i2cData + nMemAddressReadx2-1, I2C_NACK_VAL);
    i2c_master_stop(cmd);    

    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    //Error Handling
     
    //return -1 for nack, return -2 for dataCheck fail, return 0 if it is all ok
    //not going to check for transmitted data(dataCheck), but returning error numbers in case of an error
    //also not sure if i2c timeout is same as nack error but returning nack error for mlx api anyway

    if (err == ESP_ERR_TIMEOUT) {

        ESP_LOGE(TAG, "I2C Timeout");
        return -1;
    } else if (err == ESP_OK) {
        
        for(cnt=0; cnt < nMemAddressRead; cnt++)
        {
            uint16_t i = cnt << 1;
            uint16_t tmp=0;
            tmp = i2cData[i];
            tmp = tmp<<8;
            tmp |= (uint16_t)i2cData[i+1];
            data[cnt] = tmp;
        }
        //printf("nMemAddressRead : %u",nMemAddressRead);
        return 0; 
    } else {
        ESP_LOGW(TAG, "%s: Master write slave error, IO not connected....\n",
                 esp_err_to_name(err));
        return -2;
    }   

    

} 

//this function is not used anywhere inside mlx api, so it is redundant
/* void MLX90640_I2CFreqSet(int freq)
{
   // i2c.frequency(1000*freq); 
} */

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    uint8_t stack[4] = {0,0,0,0};

    stack[0] = writeAddress >> 8;
    stack[1] = writeAddress & 0x00FF;
    stack[2] = data >> 8;
    stack[3] = data & 0x00FF; 


    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, slaveAddr << 1 | I2C_MASTER_WRITE, true);

    i2c_master_write(cmd, stack, 2, true); //writing writeRegisterAddress which is 16bit (MSB - LSB)
    
    i2c_master_write(cmd, stack + 2, 2, true); //sending 16bits of data 
        
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);    

    //return -1 for nack, return -2 for dataCheck fail, return 0 if it is all ok
    //not going to check for transmitted data(dataCheck), but returning error numbers in case of an error
    //also not sure if i2c timeout is same as nack error but returning nack error for mlx api anyway

    if (err == ESP_ERR_TIMEOUT) {
        ESP_LOGE(TAG, "I2C Timeout");
        return -1;
    } else if (err == ESP_OK) {
        return 0;
    } else {
        ESP_LOGW(TAG, "%s: Master write slave error, IO not connected....\n",
                 esp_err_to_name(err));
        return -2;
    }    

}

