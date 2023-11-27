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
        #define I2C_MASTER_NUM              0                /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
        #define I2C_MASTER_FREQ_HZ          400000           /*!< I2C master clock frequency */
        #define I2C_MASTER_TX_BUF_DISABLE   0                /*!< I2C master doesn't need buffer */
        #define I2C_MASTER_RX_BUF_DISABLE   0                /*!< I2C master doesn't need buffer */
        #define I2C_MASTER_TIMEOUT_MS       1000
        #define ACK_EN                      I2C_MASTER_ACK
        #define NACK_LA                     I2C_MASTER_LAST_NACK

        //PCF8575
        #define PCF8575_SENSOR_ADDR         0x40             /*!< Slave address of the PCF8575 sensor */

        //ADS1115
        // #define ADS1115_SENSOR_ADDR_1       0x90
        // #define ADS1115_SENSOR_ADDR_2       0x92
        #define REG_Conversion              0x00
        #define REG_config		            0x01
        // #define REG_L_thresh 	            0x02
        // #define REG_H_thresh 	            0x03
        #define ADS1115_OS 		                    1 		//Operational status or single-shot conversion start
        #define ADS1115_PGA 	                    0x01	//Programmable gain amplifier configuration
        #define ADS1115_MODE                        0x00	//Device operating mode
        #define ADS1115_DR				            0x06    //Data rate
        #define	ADS1115_COMP_MODE	                0 	    //Comparator mode
        #define ADS1115_COMP_POL 	                0		//Comparator polarity
        #define ADS1115_COMP_LAT	                0 	    //Latching comparator
        #define ADS1115_COMP_QUE	                0x3	    //Comparator queue and disable

        static i2c_config_t conf;

        conf.mode = I2C_MODE_MASTER;
        conf.sda_io_num = I2C_MASTER_SDA_IO;
        conf.scl_io_num = I2C_MASTER_SCL_IO;
        conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        conf.master.clk_speed = I2C_MASTER_FREQ_HZ;

        i2c_param_config(I2C_MASTER_NUM, &conf);
        ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0));

        //PCF8675 Init
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                           
        i2c_master_start(cmd);                                                                  
        i2c_master_write_byte(cmd, PCF8575_SENSOR_ADDR | I2C_MASTER_WRITE, ACK_EN);                                              
        i2c_master_write_byte(cmd, 0xFF , ACK_EN);                                           
        i2c_master_write_byte(cmd, 0xFF , ACK_EN);                                           
        i2c_master_stop(cmd);                                                                   
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

    }

    uint16_t PCF8575_Read(){

        uint8_t read_data_1;
        uint8_t read_data_2;

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                           // 新建操作I2C句柄
        i2c_master_start(cmd);                                                                  // 启动I2C
        i2c_master_write_byte(cmd, PCF8575_SENSOR_ADDR | I2C_MASTER_READ, ACK_EN);              // 发送地址+读+检查ack
        i2c_master_read_byte(cmd, &read_data_1, ACK_EN);                                        // 读字节1
        i2c_master_read_byte(cmd, &read_data_2, NACK_LA);                                        // 读字节2
        i2c_master_stop(cmd);                                                                   // 关闭发送I2C
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        uint16_t read_data = (read_data_2 << 8) | read_data_1;

        return read_data;
    }

    void ADS1115_Write(uint8_t ADDR , uint8_t Reg , uint8_t reg_MSB , uint8_t reg_LSB){

        i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                           // 新建操作I2C句柄
        i2c_master_start(cmd);                                                                  // 启动I2C
        i2c_master_write_byte(cmd, ADDR | I2C_MASTER_WRITE, ACK_EN);             // 发送地址+写+检查ack
        i2c_master_write_byte(cmd, Reg , ACK_EN);                                               // 发送寄存器地址+检查ack
        i2c_master_write_byte(cmd, reg_MSB , ACK_EN);                                           // 发送MSB+检查ack
        i2c_master_write_byte(cmd, reg_LSB , ACK_EN);                                           // 发送LSB+检测ack
        i2c_master_stop(cmd);                                                                   // 关闭发送I2C
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

    }

    uint16_t ADS1115_Read(uint8_t ADDR){


        uint8_t read_data_1;
        uint8_t read_data_2;
        
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();                                           
        i2c_master_start(cmd);                                                                  
        i2c_master_write_byte(cmd, ADDR | I2C_MASTER_WRITE, ACK_EN);              
        i2c_master_write_byte(cmd, REG_Conversion , ACK_EN);                                              
        i2c_master_stop(cmd); 
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd); 
        i2c_master_write_byte(cmd, ADDR | I2C_MASTER_READ, ACK_EN); 
        i2c_master_read_byte(cmd, &read_data_1, ACK_EN);                                        
        i2c_master_read_byte(cmd, &read_data_2, NACK_LA);  
        i2c_master_stop(cmd);                                                                   
        i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);

        uint16_t read_data = (read_data_1 << 8) | read_data_2;
        return read_data;

    }

    void ADS1115_Setting(uint8_t ADDR, uint8_t MUX) {

        uint8_t config_MSB = (ADS1115_OS << 7) | (MUX << 4) | (ADS1115_PGA << 1) | (ADS1115_MODE);
        uint8_t config_LSB = (ADS1115_DR << 5) | (ADS1115_COMP_MODE << 4) | (ADS1115_COMP_POL << 3) | (ADS1115_COMP_LAT << 2) | (ADS1115_COMP_QUE);

        ADS1115_Write(ADDR, REG_config, config_MSB ,config_LSB);
    }
}