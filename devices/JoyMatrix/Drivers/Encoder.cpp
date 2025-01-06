#include "MatrixOS.h"
#include "Device.h"
#include "os/ui/UIcomponents.h"
#include "timers.h"
#include "Encoder.h"
#include <functional>
#include <queue>

namespace Device::Encoder
{
  StaticTimer_t encoder_timer_def;
  TimerHandle_t encoder_timer;
  QueueHandle_t encoder_evt_queue = NULL;
  std::queue<uint16_t> encoderBuff;
  EncoderEvent encoder[ENCODER_NUM];
  bool encoderTick[ENCODER_NUM];
  std::list<uint8_t> lastActive;

  bool Setup(KnobConfig* config, uint8_t n) 
  {
    if (n < ENCODER_NUM && config != nullptr)
    {
      encoder[n].Setup(config);
      return true;
    }
    return false;
  }

  bool Disable(uint8_t n) 
  {
    if (n < ENCODER_NUM)
    {
      encoder[n].knob = nullptr;
      return true;
    }
    return false;
  }

  void DisableAll() 
  {
    for (int i = 0; i < ENCODER_NUM; i++)
      Disable(i);
  }

  KnobConfig* GetEncoderPtr(uint8_t n) 
  {
    if (n < ENCODER_NUM)
      return encoder[n].knob;
    else
      return nullptr;
  }

  uint8_t GetActEncoder()  // if return 255 == no Encoder Active;
  {
    uint8_t num = 0;
    for (uint8_t n = 0; n < ENCODER_NUM; n++) 
    {
      if (encoder[n].Activated(700) == true) 
      {
        if(encoderTick[n] == false)
        {
          lastActive.push_back(n);
          encoderTick[n] = true;
        }
      }
      else if ( encoder[n].Activated(700) == false)
      {
        if(encoderTick[n] == true)
        {
          encoderTick[n] = false;
          if (lastActive.size() != 0 && lastActive.back() == n)
            lastActive.pop_back();
        }
      }
      if (encoderTick[n]) num ++;
    }
    if (num == 0 || lastActive.size() == 0) return 255;
    else return lastActive.back();  
  }

  void Scan()
  {
    while (encoderBuff.size())
    {
      for (int i = 0; i < ENCODER_NUM; i++)
      {
        if (encoder[i].knob != nullptr)
        {
          encoder[i].push(encoderBuff.front() >> i * 2 & 0x03);
        }
      }
      encoderBuff.pop();
    };

    uint8_t buffSize;
    do
    {
      buffSize = 0;
      for (int i = 0; i < ENCODER_NUM; i++)
      {
        buffSize = buffSize + encoder[i].decode();
      }
    } while (buffSize);
  }

  void Encoder_Read(void* arg) {
    uint32_t io_num;
    for (;;)
    {
      vTaskDelay(3 / portTICK_PERIOD_MS);  // debunce
      if (xQueueReceive(encoder_evt_queue, &io_num, portMAX_DELAY) == pdPASS)
      {
        encoderBuff.push(Device::I2C::PCF8574_Read());
      }
    }
  }

  void IRAM_ATTR encoder_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(encoder_evt_queue, &gpio_num, NULL);
  }

  void Init() {
    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << ENCODER_INT_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    gpio_config(&io_conf);
    encoder_evt_queue = xQueueCreate(32, sizeof(uint32_t));
    xTaskCreate(Encoder_Read, "EncoderRead", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 4, NULL);

    encoder_timer =
      xTimerCreateStatic(NULL, configTICK_RATE_HZ / Device::encoder_scanrate, true, NULL, reinterpret_cast<TimerCallbackFunction_t>(Scan), &encoder_timer_def);
    xTimerStart(encoder_timer, 0);
    // gpio_install_isr_service(0);
    gpio_isr_handler_add(ENCODER_INT_PIN, encoder_isr_handler, (void*)ENCODER_INT_PIN);
  }
}
