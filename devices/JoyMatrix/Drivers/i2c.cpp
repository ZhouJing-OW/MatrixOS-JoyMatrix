#include "MatrixOS.h"
#include "Device.h"
#include "timers.h"
#include "driver/i2c.h"




namespace Device::I2C
{
    StaticTimer_t I2C_timer_def;
    TimerHandle_t I2C_timer;

    void Init(){

        #define I2C_MASTER_SCL_IO           GPIO_NUM_37      /*!< GPIO number used for I2C master clock */
        #define I2C_MASTER_SDA_IO           GPIO_NUM_38      /*!< GPIO number used for I2C master data  */
        #define I2C_MASTER_NUM              I2C_NUM_0        /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
        #define I2C_MASTER_FREQ_HZ          400000           /*!< I2C master clock frequency */
        #define I2C_MASTER_TX_BUF_DISABLE   0                /*!< I2C master doesn't need buffer */
        #define I2C_MASTER_RX_BUF_DISABLE   0                /*!< I2C master doesn't need buffer */
        #define I2C_MASTER_TIMEOUT_MS       1000
        #define ACK_EN                      I2C_MASTER_ACK
        #define NACK_LA                     I2C_MASTER_LAST_NACK

        //PCF8575
        #define PCF8574_SENSOR_1_ADDR         0x40             /*!< Slave address of the PCF8574 sensor */
        #define PCF8574_SENSOR_2_ADDR         0x42

        #define REG_Conversion              0x00
        #define REG_config		            0x01
        // #define REG_L_thresh 	            0x02
        // #define REG_H_thresh 	            0x03


        static i2c_config_t conf;

        conf.mode = I2C_MODE_MASTER;
        conf.sda_io_num = I2C_MASTER_SDA_IO;
        conf.scl_io_num = I2C_MASTER_SCL_IO;
        conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

        i2c_param_config(I2C_MASTER_NUM, &conf);
        ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));

        //PCF8674 Init
        
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                           
        i2c_master_start(cmd);                                                                  
        i2c_master_write_byte(cmd, PCF8574_SENSOR_1_ADDR | I2C_MASTER_WRITE, ACK_EN);                                              
        i2c_master_write_byte(cmd, 0xFF , ACK_EN);                                         
        i2c_master_stop(cmd);                                                                   
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();                                           
        i2c_master_start(cmd2);                                                                  
        i2c_master_write_byte(cmd2, PCF8574_SENSOR_2_ADDR | I2C_MASTER_WRITE, ACK_EN);                                              
        i2c_master_write_byte(cmd2, 0xFF , ACK_EN);                                         
        i2c_master_stop(cmd2);                                                                   
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd2);

    }

    uint16_t PCF8574_Read(){

        uint8_t read_data_1 ;
        uint8_t read_data_2 ;

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                           // 新建操作I2C句柄
        i2c_master_start(cmd);                                                                  // 启动I2C
        i2c_master_write_byte(cmd, PCF8574_SENSOR_1_ADDR | I2C_MASTER_READ, ACK_EN);              // 发送地址+读+检查ack
        i2c_master_read_byte(cmd, &read_data_1, NACK_LA);                                        // 读字节1
        i2c_master_stop(cmd);                                                                   // 关闭发送I2C
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();                                           // 新建操作I2C句柄
        i2c_master_start(cmd2);                                                                  // 启动I2C
        i2c_master_write_byte(cmd2, PCF8574_SENSOR_2_ADDR | I2C_MASTER_READ, ACK_EN);              // 发送地址+读+检查ack
        i2c_master_read_byte(cmd2, &read_data_2, NACK_LA);                                        // 读字节1
        i2c_master_stop(cmd2);                                                                   // 关闭发送I2C
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd2, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd2);

        uint16_t read_data = (read_data_2 << 8) | read_data_1;
        
        return read_data;

    }

}