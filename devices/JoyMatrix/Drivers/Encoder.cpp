#include <queue>
#include <functional>
#include "MatrixOS.h"
#include "Device.h"
#include "timers.h"
#include "Encoder.h"

namespace Device::Encoder
{
    StaticTimer_t encoder_timer_def;
    TimerHandle_t encoder_timer;
    QueueHandle_t encoder_evt_queue = NULL;
    std::queue<uint16_t> encoderBuff;
    EncoderEvent encoder[ENCODER_NUM];

    bool Setup(KnobConfig *config, uint8_t n){
        if(n < ENCODER_NUM) {
          encoder[n].setup(config);
          return true;
        }
        return false;
    }

    bool Disable(uint8_t n){
        if(encoder[n].knob != nullptr && n < ENCODER_NUM) {
          encoder[n].knob->enable = false;
          return true;
        }
        return false;
    }

    void DisableAll(){
        for(int i = 0; i < ENCODER_NUM; i++){
            Disable(i);
        }
    }

    KnobConfig* GetEncoderKnob(uint8_t n){
        return encoder[n].knob;
    }

    void ReadBuff() {
        while(encoderBuff.size()){
            for(int i = 0; i < ENCODER_NUM; i++){
                if (encoder[i].knob != nullptr && encoder[i].knob->enable == true){
                    encoder[i].push(encoderBuff.front() >> i * 2 & 0x03);
                }
            }
            encoderBuff.pop();
        };

        uint8_t buffSize;
        do {
          buffSize = 0;
          for(int i = 0; i < ENCODER_NUM; i++){
              buffSize = buffSize + encoder[i].decode();
          }
        } while (buffSize);
    }

    void Encoder_Read(void* arg){
        uint32_t io_num;
        for(;;) {
            if(xQueueReceive(encoder_evt_queue, &io_num, portMAX_DELAY)==pdPASS){
                encoderBuff.push(Device::I2C::PCF8574_Read());
            }
        }
    }
    
    void IRAM_ATTR encoder_isr_handler(void* arg){
        uint32_t gpio_num = (uint32_t) arg;
        xQueueSendFromISR(encoder_evt_queue, &gpio_num, NULL);
    }

    void Init(){
        gpio_config_t io_conf;
        io_conf.mode = GPIO_MODE_INPUT; //输入模式
        io_conf.pin_bit_mask = ( 1ULL<<ENCODER_INT_PIN ); //配置引脚
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE; //引脚电平上拉
        io_conf.intr_type = GPIO_INTR_NEGEDGE; //下降沿中断
        //配置gpio/
        gpio_config (&io_conf);

        encoder_timer =
            xTimerCreateStatic(NULL, configTICK_RATE_HZ / Device::encoder_scanrate, true, NULL, reinterpret_cast<TimerCallbackFunction_t>(ReadBuff), &encoder_timer_def);
        xTimerStart(encoder_timer, 0);
        encoder_evt_queue = xQueueCreate(10, sizeof(uint32_t));
        xTaskCreate(Encoder_Read, "EncoderRead", 2048, NULL, configMAX_PRIORITIES - 1, NULL);
        // gpio_install_isr_service(0); //注册中断服务
        gpio_isr_handler_add(ENCODER_INT_PIN, encoder_isr_handler, (void*) ENCODER_INT_PIN);
    }
}
