#include "MatrixOS.h"
#include "Device.h"
#include "timers.h"

namespace Device::Encoder
{
    StaticTimer_t encoder_timer_def;
    TimerHandle_t encoder_timer;
    QueueHandle_t encoder_evt_queue = NULL;

    #define ENCODER_INT_PIN         GPIO_NUM_36
    #define ENCODER_BUFF_MAX        255

    uint16_t readDataBuff[ENCODER_BUFF_MAX] = {0xFF};
    uint8_t buffNum = 0;
    uint8_t scanNum = 0;
    uint8_t cycle = 0;

    bool encoderPin[16];
    uint8_t encoder[8];
    uint8_t EveCnt[8];
    uint8_t num[8];
    Color* color[8];
    uint8_t type[8];
    uint8_t ch[8];
    uint8_t val1[8];
    int32_t* valuePtr[8];
    int32_t min[8];
    int32_t max[8];
    int32_t defaultValue[8];
    uint8_t activeKnob;

    void Setup(Color* ColorInput, uint8_t Type , uint8_t Ch , uint8_t Val , int32_t* Ptr, int32_t Min ,int32_t Max, int32_t Default, uint8_t n){
      color[n] = ColorInput;
      type[n] = Type;
      ch[n] = Ch;
      val1[n] = Val;
      valuePtr[n] = Ptr;
      min[n] = Min;
      max[n] = Max;
      defaultValue[n] = Default;
    }


    void Scan() {

        for (; scanNum < buffNum + (cycle * ENCODER_BUFF_MAX); scanNum++)
        {
            if (scanNum >= ENCODER_BUFF_MAX){
                scanNum = 0;
                cycle--;
            };

            for (uint8_t i = 0; i < 16; i++){
                encoderPin[i] = bitRead(readDataBuff[scanNum], i);
            }

            for (uint8_t n = 0; n < 8; n++){

                uint8_t m = (encoderPin[7 - n] << 1 | encoderPin[8 + n]);

                switch (EveCnt[n])
                {
                case 0:  // 保持状态0(口线保持信息为00)
                    if (m != 0)
                    {
                        if (m == 1)  // 读入信息为01，表示正转
                        {
                            EveCnt[n] = 1;
                            encoder[n] = 1;
                            activeKnob = n;
                            if ( type[n] != 0 && valuePtr[n] != nullptr && *valuePtr[n] < max[n]){
                                if (*valuePtr[n] < max[n] - (Device::rightShift | Device::leftShift) * 3){
                                  *valuePtr[n] = *valuePtr[n] + 1 + (Device::rightShift | Device::leftShift) * 3;
                                } else {
                                  (*valuePtr[n])++;
                                }
                                knobFunction(n);
                                  // MLOGD("Encoder", "Printing %d", *valuePtr[n]);
                            };
                        }
                        if (m == 2)  // 读入信息为10，表示反转
                        {
                            EveCnt[n] = 1;
                            encoder[n] = 2;
                            activeKnob = n;
                            if (type[n] != 0 && valuePtr[n] != nullptr && *valuePtr[n]> min[n]){
                                if (*valuePtr[n] - (Device::rightShift | Device::leftShift) * 3 > min[n] ){
                                  *valuePtr[n] = *valuePtr[n] - 1 - (Device::rightShift | Device::leftShift) * 3;
                                } else {
                                  (*valuePtr[n])--;
                                }
                              knobFunction(n);
                              //MLOGD("Encoder", "Printing %d", *valuePtr[n]);
                            };
                        }
                        if (m == 3)
                        {
                            EveCnt[n] = 2;
                        }
                    }
                    break;
                case 1:              // 等待保持状态切换到1
                    if (m == 0x03)
                    EveCnt[n] = 2;  // 进入保持状态1
                    break;
                case 2:              // 保持状态1（口线保持信息为11）
                    if (m != 3)
                    {
                        if (m == 1)
                        {
                            EveCnt[n] = 3;  // 读入信息为01，表示反转
                            encoder[n] = 2;
                            activeKnob = n;
                            if (buffNum + (cycle * ENCODER_BUFF_MAX) - scanNum > 4 && type[n] != 0 && valuePtr[n] != nullptr && *valuePtr[n] > min[n]   && max[n]-min[n] > 64){
                              if (*valuePtr[n] - (Device::rightShift | Device::leftShift) * 3 > min[n] ){
                                  *valuePtr[n] = *valuePtr[n] - 1 - (Device::rightShift | Device::leftShift) * 3;
                                } else {
                                  (*valuePtr[n])--;
                                }
                              knobFunction(n);
                              //MLOGD("Encoder", "Printing %d", *valuePtr[n]);
                            };
                        }
                        if (m == 2)
                        {
                            EveCnt[n] = 3;  // 读入信息为10，表示正转
                            encoder[n] = 1;
                            activeKnob = n;
                            if (buffNum + (cycle * ENCODER_BUFF_MAX) - scanNum > 4 &&type[n] != 0 && valuePtr[n] != nullptr && *valuePtr[n] < max[n] && max[n]-min[n] > 64){
                                if (*valuePtr[n] < max[n] - (Device::rightShift | Device::leftShift) * 3){
                                  *valuePtr[n] = *valuePtr[n] + 1 + (Device::rightShift | Device::leftShift) * 3;
                                } else {
                                  (*valuePtr[n])++;
                                }
                              knobFunction(n);
                              //MLOGD("Encoder", "Printing %d", *valuePtr[n]);
                            };
                        }
                        if(m == 0)
                        {
                            EveCnt[n] = 0;
                        }
                    }
                    break;
                case 3:              // 对读入信息11进行滤波处理
                    if (m == 0x00)
                    EveCnt[n] = 0;  // 滤波成功表示保持切换，进入保持状态0
                    break;
                default:
                    break;
                }
            }
        }

    }

    Color* GetColor(uint8_t n){
        return color[n];
    }

    int32_t* GetValue(uint8_t n){
        return valuePtr[n];
    }

    int32_t* GetDefault(uint8_t n){
        return &defaultValue[n];
    }

    uint8_t* GetActiveKnob(){
        return &activeKnob;
    }

    void Encoder_Read(void* arg){

        uint32_t io_num;
        for(;;) {

            if(xQueueReceive(encoder_evt_queue, &io_num, portMAX_DELAY)==pdPASS) {

                if (buffNum >= ENCODER_BUFF_MAX){
                  buffNum = 0;
                  cycle++;
                };

                readDataBuff[buffNum] = Device::I2C::PCF8575_Read();
                buffNum++;

            }
        }
    }

    
    void IRAM_ATTR encoder_isr_handler(void* arg)
    {

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

        encoder_timer = xTimerCreateStatic(NULL, configTICK_RATE_HZ / Device::encoder_scanrate, true, NULL, reinterpret_cast<TimerCallbackFunction_t>(Scan), &encoder_timer_def);
        xTimerStart(encoder_timer, 0);

        encoder_evt_queue = xQueueCreate(10, sizeof(uint32_t));
        xTaskCreate(Encoder_Read, "EncoderRead", 2048, NULL, configMAX_PRIORITIES - 1, NULL);
        // gpio_install_isr_service(0); //注册中断服务
        gpio_isr_handler_add(ENCODER_INT_PIN, encoder_isr_handler, (void*) ENCODER_INT_PIN);

    }

    void knobFunction(uint8_t n){
        switch(type[n]){
            case 0 :// Do nothing
            {
                break;
            }
            case 1 :// CC message
            {   
                MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, ch[n], val1[n], *valuePtr[n]));
                break;
            }
            case 2 :// System
            {
                break; 
            }
        }
    }

}
