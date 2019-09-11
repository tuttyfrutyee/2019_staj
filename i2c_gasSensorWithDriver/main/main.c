#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"

#include "bme680.h"

#define I2C_MASTER_SCL_IO GPIO_NUM_19               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO_NUM_18               /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM 1                            /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000                   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                 /*!< I2C master doesn't need buffer */


#define BME680_SENSOR_ADDR 0x76                     /*!< slave address for BH1750 sensor */
#define BME680_CMD_START 0x74                      /*!< Operation mode  0x22(MSB) - 0X23(LSB) - 0X24(XLSB) */
#define WRITE_BIT 0                                 /*!< I2C master write */
#define READ_BIT 1                                  /*!< I2C master read */
#define ACK_CHECK_EN 0x1                            /*!< I2C master will check ack from slave*/
#define ACK_VAL 0x0                                 /*!< I2C ack value */
#define NACK_VAL 0x1                                /*!< I2C nack value */
#define I2C_ACK_VAL ACK_VAL
#define I2C_NACK_VAL NACK_VAL


static const char *TAG = "i2c_gasSensor";

void user_delay_ms(uint32_t period)
{
    /*
     * Return control or wait,
     * for a period amount of milliseconds
     */
    vTaskDelay(period / portTICK_RATE_MS);
}

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init()
{
    int i2c_master_port = I2C_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    i2c_param_config(i2c_master_port, &conf);
    return i2c_driver_install(i2c_master_port, conf.mode,
                              I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);
}



int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    if (len == 0) return true;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (true)
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( dev_id << 1 ) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, reg_addr, true);
        if (!reg_data)
            i2c_master_stop(cmd);
    }
    if (true)
    {
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, ( dev_id << 1 ) | I2C_MASTER_READ, true);
        if (len > 1) i2c_master_read(cmd, reg_data, len-1, I2C_ACK_VAL);
        i2c_master_read_byte(cmd, reg_data + len-1, I2C_NACK_VAL);
        i2c_master_stop(cmd);
    }
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

return err;
}

int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, dev_id << 1 | I2C_MASTER_WRITE, true);

    i2c_master_write_byte(cmd, reg_addr, true);
    
    i2c_master_write(cmd, reg_data, len, true);
        
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
return err;
}

void app_main(void)
{

    i2c_master_init();


    int ret;



    uint8_t allRegisters[255] = {0};

/*     uint8_t data[2] = {0x44,0x55};
    user_i2c_write(BME680_SENSOR_ADDR, BME680_CMD_START, data, 2);
    vTaskDelay(1500 / portTICK_RATE_MS); */

/*     ret = user_i2c_read(BME680_SENSOR_ADDR, 0X00, allRegisters, 256);

    for(int i = 0; i < 256; i++){
        printf("allRegisters[%d] : 0x%02x\n",i,allRegisters[i]);
    } */


    struct bme680_dev gas_sensor;

    gas_sensor.dev_id = BME680_I2C_ADDR_PRIMARY;
    gas_sensor.intf = BME680_I2C_INTF;
    gas_sensor.read = user_i2c_read;
    gas_sensor.write = user_i2c_write;
    gas_sensor.delay_ms = user_delay_ms;
    /* amb_temp can be set to 25 prior to configuring the gas sensor 
     * or by performing a few temperature readings without operating the gas sensor.
     */
    gas_sensor.amb_temp = 25;


    int8_t rslt = BME680_OK;
    rslt = bme680_init(&gas_sensor);






    uint8_t set_required_settings;

    /* Set the temperature, pressure and humidity settings */
    gas_sensor.tph_sett.os_hum = BME680_OS_2X;
    gas_sensor.tph_sett.os_pres = BME680_OS_4X;
    gas_sensor.tph_sett.os_temp = BME680_OS_8X;
    gas_sensor.tph_sett.filter = BME680_FILTER_SIZE_3;

    /* Set the remaining gas sensor settings and link the heating profile */
    gas_sensor.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
    /* Create a ramp heat waveform in 3 steps */
    gas_sensor.gas_sett.heatr_temp = 320; /* degree Celsius */
    gas_sensor.gas_sett.heatr_dur = 150; /* milliseconds */

    /* Select the power mode */
    /* Must be set before writing the sensor configuration */
    gas_sensor.power_mode = BME680_FORCED_MODE; 

    /* Set the required sensor settings needed */
    set_required_settings = BME680_OST_SEL | BME680_OSP_SEL | BME680_OSH_SEL | BME680_FILTER_SEL 
        | BME680_GAS_SENSOR_SEL;

    /* Set the desired sensor configuration */
    rslt = bme680_set_sensor_settings(set_required_settings,&gas_sensor);

    /* Set the power mode */
    rslt = bme680_set_sensor_mode(&gas_sensor);






    /* Get the total measurement duration so as to sleep or wait till the
     * measurement is complete */
    uint16_t meas_period;
    bme680_get_profile_dur(&meas_period, &gas_sensor);

    struct bme680_field_data data;
    int counter = 100;
    while(1)
    {
        user_delay_ms(meas_period); /* Delay till the measurement is ready */

        rslt = bme680_get_sensor_data(&data, &gas_sensor);

        printf("T: %.2f degC, P: %.2f hPa, H %.2f %%rH ", data.temperature / 100.0f,
            data.pressure / 100.0f, data.humidity / 1000.0f );
        /* Avoid using measurements from an unstable heating setup */
        if(data.status & BME680_GASM_VALID_MSK)
            printf(", G: %d ohms", data.gas_resistance);

        printf("\r\n");

        /* Trigger the next measurement if you would like to read data out continuously */
        if (gas_sensor.power_mode == BME680_FORCED_MODE) {
            rslt = bme680_set_sensor_mode(&gas_sensor);
        }

    }





// Test for reading and writing to slave



/*     uint8_t data[2] = {0x44,0x55};
    user_i2c_write(BME680_SENSOR_ADDR, BME680_CMD_START, data, 2);
    vTaskDelay(1500 / portTICK_RATE_MS); */

/*     ret = user_i2c_read(BME680_SENSOR_ADDR, 0X00, allRegisters, 256);

    for(int i = 0; i < 256; i++){
        printf("allRegisters[%d] : 0x%02x\n",i,allRegisters[i]);
    } */
}

