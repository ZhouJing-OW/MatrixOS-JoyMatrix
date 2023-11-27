// Define Device Keypad Function
#include "MatrixOS.h"
#include "Device.h"
#include "timers.h"

namespace Device::KeyPad
{
  StaticTimer_t keypad_timer_def;
  TimerHandle_t keypad_timer;

  void Init() {
    LoadCustomSettings();
    InitFN();
    InitKeyPad();
    // InitTouchBar();
  }

  // TODO Change to use interrupt
  void InitFN() {
    gpio_config_t io_conf;

    // Config FN
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pin_bit_mask = (1ULL << fn_pin);
#ifdef FN_PIN_ACTIVE_HIGH  // Active HIGH
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
#else  // Active Low
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
#endif
    gpio_config(&io_conf);

    // Set up Matrix OS key config
    fnState.setConfig(&fn_config);
  }

  void InitKeyPad() {
    if (!velocity_sensitivity)
    { Binary::Init(); }
    else
    { FSR::Init(); }
  }

  void StartKeyPad() {
    if (!velocity_sensitivity)
    {
      Binary::Start();
    }
    else
    {
      FSR::Start();
    }

    keypad_timer = xTimerCreateStatic(NULL, configTICK_RATE_HZ / Device::keypad_scanrate, true, NULL, reinterpret_cast<TimerCallbackFunction_t>(Scan), &keypad_timer_def);
    

    xTimerStart(keypad_timer, 0);
  }

  void Start() {
    StartKeyPad();
    // StartTouchBar();
  }

  void Scan() {
    ScanFN();
    ScanKeyPad();
  }

  bool ScanKeyPad() {
    if (!velocity_sensitivity)
    { return Binary::Scan(); }
    else
    { return FSR::Scan(); }
  }

  bool ScanFN() {
    Fract16 read = gpio_get_level(fn_pin) * UINT16_MAX;
    // ESP_LOGI("FN", "%d", gpio_get_level(fn_pin));
    if (fn_active_low)
    { read = UINT16_MAX - (uint16_t)read; }
    if (fnState.update(read, false))
    {
      if (NotifyOS(0, &fnState))
      { return true; }
    }
    return false;
  }

  bool ShiftActived() {
    bool actived = (Device::KeyPad::GetKey(RSHIFT_KEY)->active()) | (Device::KeyPad::GetKey(LSHIFT_KEY)->active());
    return actived;
  }

  bool AltActived() {
    bool actived = (Device::KeyPad::GetKey(RALT_KEY)->active()) | (Device::KeyPad::GetKey(LALT_KEY)->active());
    return actived;
  }

  void Clear() {
    fnState.Clear();
    RShiftState.Clear();
    LShiftState.Clear();
    LAltState.Clear();
    RAltState.Clear();

    for (uint8_t x = 0; x < x_size; x++)
    {
      for (uint8_t y = 0; y < y_size; y++)
      { if(!MatrixOS::MIDI::CheckHoldingNote(Point(x,y)))
        keypadState[x][y].Clear(); }
    }

    // for (uint8_t i = 0; i < touchbar_size; i++)
    // { touchbarState[i].Clear(); }
  }

  KeyInfo* GetKey(uint16_t keyID) {
    uint8_t keyClass = keyID >> 12;
    switch (keyClass)
    {
      case 0:  // System
      {
        uint16_t index = keyID & (0b0000111111111111);
        switch (index)
        {
          case 0:
            return &fnState;
          case 1:
            return &RShiftState;
          case 2:
            return &LShiftState;
          case 3:
            return &LAltState;
          case 4:
            return &RAltState;
        }
        break;
      }
      case 1:  // Main Grid
      {
        int16_t x = (keyID & (0b0000111111000000)) >> 6;
        int16_t y = keyID & (0b0000000000111111);
        if (x < x_size && y < y_size)
          return &keypadState[x][y];
        break;
      }
      // case 2:  // Touch Bar
      //  {
      //  uint16_t index = keyID & (0b0000111111111111);
      //  MLOGD("Keypad", "Read Touch %d", index);
      //  if (index < touchbar_size)
      //    return &touchbarState[index];
      //  break;
      // }
    }
    return nullptr;  // Return an empty KeyInfo
  }
  bool NotifyOS(uint16_t keyID, KeyInfo* keyInfo) {
    KeyEvent keyEvent;
    keyEvent.id = keyID;
    keyEvent.info = *keyInfo;
    return MatrixOS::KEYPAD::NewEvent(&keyEvent);
  }

  uint16_t XY2ID(Point xy) {
    if (xy.x >= 0 && xy.x < 16 && xy.y >= 0 && xy.y < 8)  // Main grid
    { return (1 << 12) + (xy.x << 6) + xy.y; }
    // else if ((xy.x == -1 || xy.x == 8) && (xy.y >= 0 && xy.y < 8))  // Touch Bar
    // { return (2 << 12) + xy.y + (xy.x == 8) * 8; }
    // MLOGE("Keypad", "Failed XY2ID %d %d", xy.x, xy.y);
    return UINT16_MAX;
  }

  // Matrix use the following ID Struct
  // CCCC IIIIIIIIIIII
  // C as class (4 bits), I as index (12 bits). I could be splited by the class defination, for example, class 0 (grid),
  // it's splited to XXXXXXX YYYYYYY. Class List: Class 0 - System - IIIIIIIIIIII Class 1 - Grid - XXXXXX YYYYYY Class 2
  // - TouchBar - IIIIIIIIIIII Class 3 - Underglow - IIIIIIIIIIII

  Point ID2XY(uint16_t keyID) {
    uint8_t keyClass = keyID >> 12;
    switch (keyClass)
    {
      case 1:  // Main Grid
      {
        int16_t x = (keyID & 0b0000111111000000) >> 6;
        int16_t y = keyID & (0b0000000000111111);
        if (x < Device::x_size && y < Device::y_size)
          return Point(x, y);
        break;
      }
      /*
      case 2:  // TouchBar
      {
        uint16_t index = keyID & (0b0000111111111111);
        if (index < Device::touchbar_size)
        {
          if (index / 8)  // Right
          { return Point(Device::x_size, index % 8); }
          else  // Left
          { return Point(-1, index % 8); }
        }
        break;
      }*/
    }
    return Point(INT16_MIN, INT16_MIN);
  }

  uint8_t GetVelocity(){
    uint8_t velocity;
    if (Device::KeyPad::pressureSensitive == true)
    {
      uint8_t range = Device::KeyPad::velocity_max - Device::KeyPad::velocity_min;
      uint8_t devicePressure = Device::KeyPad::LPressure >= Device::KeyPad::RPressure ? Device::KeyPad::LPressure : Device::KeyPad::RPressure;
      velocity = devicePressure * range / 127 + Device::KeyPad::velocity_min;
    }
    else
    {
      velocity = Device::KeyPad::velocity_def;
    }
    return velocity;
  }
}