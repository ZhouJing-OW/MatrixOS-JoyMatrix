#include <queue>
#include <functional>
#include "MatrixOS.h"
#include "Device.h"
#include "timers.h"


namespace Device::Encoder
{
    StaticTimer_t encoder_timer_def;
    TimerHandle_t encoder_timer;
    QueueHandle_t encoder_evt_queue = NULL;

    #define ENCODER_INT_PIN GPIO_NUM_36

    std::queue<uint16_t> encoderBuff;

    class EncoderEvent
    {
    public:
        std::queue<uint8_t> pin;
        KnobConfig *knob = nullptr;
        int16_t* val;
        uint8_t state;

        void setup(int16_t *val, KnobConfig *config){
            this->knob = config;
            this->val = val;
            if (knob->min < knob->max){
              if (*val < knob->min) { *val = knob->min; }
              if (*val > knob->max) { *val = knob->max; }
              knob->enable = true;
            } else
              knob->enable = false;
        }

        void disable(){
            if (knob != nullptr) {
              knob->enable = false;
            }
        }

        void push(uint8_t pin) {
            if(this->pin.size() == 0 || (this->pin.back() != pin)){
                this->pin.push(pin); 
            }
        }

        bool decode(){
          if (pin.size() > 0)
          {
            uint8_t m = pin.front();
            int16_t min = knob->min;
            int16_t max = knob->max;
            bool shift = Device::KeyPad::ShiftActived();
            bool wide = max - min > 24;
            const uint8_t hs = 4;
            switch (state)
            {
              case 0:  // 00 filter
                if (m != 0){
                  state = m;
                }
                break;
              case 1:  // 01
                if (m != 1){
                  if (m == 0){
                    state = 0;
                    if (*val + hs < max && wide && !shift) {
                      *val = *val + hs;
                      Callback();
                    }
                    if (pin.size() > hs && shift) {
                      *val = *val + 1;
                      Callback();
                    }
                  } else
                    state = m;
                }
                break;
              case 2:  // 10
                if (m != 2){
                  if (m == 0){
                    state = 0;
                    if (*val - hs >= min && wide && !shift) {
                      *val = *val - hs;
                      Callback();
                    }
                    if (pin.size() > hs && shift) {
                      *val = *val - 1;
                      Callback();
                    }
                  } else
                    state = m;
                }
                break;
              case 3:  // 11
                if (m != 3){
                  if (m == 1){
                    state = 1;  // 读入信息为01，表示正转
                    if (*val < max){
                      if (*val + hs <= max && wide && !shift) *val = *val + hs;
                      else *val = *val + 1;
                      Callback();
                    }
                  }
                  if (m == 2){
                    state = 2;  // 读入信息为10，表示反转
                    if (*val > min){
                      if (*val - hs >= min && wide && !shift) *val = *val - hs;
                      else *val = *val - 1;
                      Callback();
                    }
                  }
                  if (m == 0){
                    state = 0;
                  }
                }
                break;
            }
            pin.pop();
            return true;
            }
            return false;
        }

        void Callback() {
            MatrixOS::Component::Knob_Function(knob, val);
        }
    };

    EncoderEvent encoder[8];

    void Setup(int16_t *val, KnobConfig *config, uint8_t n){
        if(n < 8) encoder[n].setup(val, config);
    }

    void Disable(uint8_t n){
        if(n < 8) encoder[n].disable();
    }

    void DisableAll(){
        for(int i = 0; i < 8; i++){
            encoder[i].disable();
        }
    }

    KnobConfig* GetConfig(uint8_t n){
        return encoder[n].knob;
    }

    void ReadBuff() {
        while(encoderBuff.size()){
            for(int i = 0; i < 8; i++){
                if (encoder[i].knob != nullptr && encoder[i].knob->enable){
                    encoder[i].push(encoderBuff.front() >> i * 2 & 0x03);
                }
            }
            encoderBuff.pop();
        };

        uint8_t buffSize;
        do {
          buffSize = 0;
          for(int i = 0; i < 8; i++){
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
        //配置gpio
        gpio_config (&io_conf);
        encoder_timer = xTimerCreateStatic(NULL, configTICK_RATE_HZ / Device::encoder_scanrate, true, NULL, reinterpret_cast<TimerCallbackFunction_t>(ReadBuff), &encoder_timer_def);
        xTimerStart(encoder_timer, 0);
        encoder_evt_queue = xQueueCreate(10, sizeof(uint32_t));
        xTaskCreate(Encoder_Read, "EncoderRead", 2048, NULL, configMAX_PRIORITIES - 1, NULL);
        // gpio_install_isr_service(0); //注册中断服务
        gpio_isr_handler_add(ENCODER_INT_PIN, encoder_isr_handler, (void*) ENCODER_INT_PIN);
    }
}
