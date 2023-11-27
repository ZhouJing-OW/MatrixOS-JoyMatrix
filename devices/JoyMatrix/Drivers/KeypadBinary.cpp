#include "MatrixOS.h"
#include "Device.h"
// #include "ulp_keypad.h"
#include "ulp_riscv.h"

// extern const uint8_t ulp_keypad_bin_start[] asm("_binary_ulp_keypad_bin_start");
// extern const uint8_t ulp_keypad_bin_end[] asm("_binary_ulp_keypad_bin_end");

namespace Device::KeyPad::Binary
{
  void Init() {
    gpio_config_t io_conf;

    // Config Input Pins
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = 0;
    for (uint8_t y = 0; y < y_size; y++)
    { io_conf.pin_bit_mask |= (1ULL << keypad_read_pins[y]); }
    gpio_config(&io_conf);

    // Config FuncInput Pins
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = 0;
    io_conf.pin_bit_mask |= (1ULL << keypad_funcRead_pins[0]);
    gpio_config(&io_conf);
    
    // Config Output Pins
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pin_bit_mask = 0;
    for (uint8_t x = 0; x < 4; x++)
    { io_conf.pin_bit_mask |= (1ULL << keypad_write_pins[x]); }
    gpio_config(&io_conf);



    // Set all output pins to low
    for (uint8_t x = 0; x < 4; x++)
    { gpio_set_level(keypad_write_pins[x], 0); }

    for (uint8_t x = 0; x < x_size; x++)
    {
      for (uint8_t y = 0; y < y_size; y++)
      { keypadState[x][y].setConfig(&keypad_config); }
      
      if(x<4)
      {
        switch (x)
        {
          case 0:
            RShiftState.setConfig(&keypad_config);
            break;
          case 1:
            LShiftState.setConfig(&keypad_config);
            break;
          case 2:
            LAltState.setConfig(&keypad_config);
            break;
          case 3:
            RAltState.setConfig(&keypad_config);
            break;
        }
      }
    }
  }

  void Start()
  {
    // ulp_riscv_load_binary(ulp_keypad_bin_start, (ulp_keypad_bin_end - ulp_keypad_bin_start));
    // ulp_riscv_run();
  }

  // bool Scan()
  // {
  //   // ESP_LOGI("Keypad ULP", "Scaned: %lu", ulp_count);
  //   uint16_t (*result)[8] = (uint16_t(*)[8])&ulp_result;
  //   for(uint8_t y = 0; y < Device::y_size; y ++)
  //   {
  //     for(uint8_t x = 0; x < Device::x_size; x++)
  //     {
  //       Fract16 reading = (result[x][y] > 0) * FRACT16_MAX;
  //       bool updated = keypadState[x][y].update(reading, true);
  //       if (updated)
  //       {
  //         uint16_t keyID = (1 << 12) + (x << 6) + y;
  //         if (NotifyOS(keyID, &keypadState[x][y]))
  //         {           
  //           return true; 
  //         }
  //       }
  //     }
  //   }
  //   return false;
  // }

  bool Scan()
  {
    for(uint8_t x = 0; x < x_size; x++)
    {
      //TODO 需要更好的代码提高gpio翻转速度
      //gpio_set_level(keypad_write_pins[3], x < 8 ? 0 : 1);

      for(uint8_t a = 0; a < 4; a++) {
        gpio_set_level(keypad_write_pins[a], (x >> a) & 1);
      }

      for(uint8_t a = 0; a < 4; a++) {
        gpio_set_level(keypad_write_pins[a], (x >> a) & 1);
      }


      for(uint8_t y = 0; y < y_size; y++)
      {
        Fract16 reading = gpio_get_level(keypad_read_pins[y]) * FRACT16_MAX;
        // MLOGD("Keypad", "%d %d Read: %d", x, y, gpio_get_level(keypad_read_pins[y]));

        bool updated = keypadState[x][y].update(reading, true); 
        if (updated)
        {
          uint16_t keyID = (1 << 12) + (x << 6) + y;
          if (NotifyOS(keyID, &keypadState[x][y]))
          {           
            return true; 
          }
        }     
      }

      if (x < 4) // RShift LShift RAlt LAlt
      { 
        Fract16 reading = gpio_get_level(keypad_funcRead_pins[0]) * FRACT16_MAX;
        switch (x)
        {
          case 0:
          {
            bool updated = RShiftState.update(reading, false);
            if (updated){
              if (NotifyOS(1, &RShiftState))
              {
                return true;
              }
            }
            break;
          }
          case 1:
          {
            bool updated = LShiftState.update(reading, false);
            if (updated){
              if (NotifyOS(2, &LShiftState))
              {
                return true;
              }
            }
            break;
          }
          case 2:
          {
            bool updated = LAltState.update(reading, false);
            if (updated){
              if (NotifyOS(3, &LAltState))
              {
                return true;
              }
            }
            break;
          }
          case 3:
          {
            bool updated = RAltState.update(reading, false);
            if (updated){
              if (NotifyOS(4, &RAltState))
              {
                return true;
              }
            }
            break;
          }
        }
      }
    }
    return false;
  }
}