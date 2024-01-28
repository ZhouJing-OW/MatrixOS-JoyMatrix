#pragma once

#include "framework/Framework.h"
#include "tusb.h"
#include "Config.h"

namespace Device
{
  /*
  Required Varaiables:
  const string name;
  const uint16_t numsOfLED;
  const uint8_t x_size;
  const uint8_t y_size;
  string serial_number;
  */

  void DeviceInit();
  void DeviceStart();
  void Reboot();
  void Bootloader();
  void ErrorHandler();

  void Log(string format, va_list valst);

  string GetSerial();

  namespace LED
  {
    void Update(Color* frameBuffer, uint8_t brightness = 255);  // Render LED
    uint16_t XY2Index(Point xy);        // Grid XY to global buffer index, return UINT16_MAX if not index for given XY
    uint16_t ID2Index(uint16_t ledID);  // Local led Index to buffer index, return UINT16_MAX if not index for given
                                        // Index
  }

  namespace KeyPad
  {
    KeyInfo* GetKey(uint16_t keyID);
    void Clear();  // Since only Device layer awares the keyInfo buffer, the function's job is to run Clear() on all
                   // keyInfo
    uint16_t XY2ID(Point xy);  // Not sure if this is required by Matrix OS, added in for now. return UINT16_MAX if no
                               // ID is assigned to given XY
    Point ID2XY(uint16_t keyID);  // Locate XY for given key ID, return Point(INT16_MIN, INT16_MIN) if no XY found for
                                  // given ID;
  }

    namespace Encoder
  {
    void Init();
    bool Setup(KnobConfig *config, uint8_t n);
    bool Disable(uint8_t n);
    void DisableAll();
    KnobConfig* GetEncoderPtr(uint8_t n);
    KnobConfig* GetDialPtr(uint8_t n);
    uint8_t GetActEncoder();
  }

    namespace AnalogInput
  {
    void Init();
    void UseDial(Point xy, KnobConfig* knob, std::function<void()> callback = nullptr);
    KnobConfig* GetDialPtr();
    void SetUpDown(int8_t* up_down, int8_t max, int8_t min = 0, int8_t step = 1, bool loop = false, std::function<void()> callback = nullptr);
    void SetLeftRight(int8_t* left_right, int8_t max, int8_t min = 0, int8_t step = 1, bool loop = false, std::function<void()> callback = nullptr);
    void DisableDirectPad();
    int8_t* GetUDPtr();
    int8_t* GetLRPtr();
    bool UDIsLonger(uint32_t ms);
    bool LRIsLonger(uint32_t ms);
  }

    namespace FATFS
  {
    void Init();
    void Format();
  }

  // namespace BKP  // Back up register, presistant ram after software reset.
  // {
  //   extern uint16_t size;
  //   uint32_t Read(uint32_t address);
  //   int8_t Write(uint32_t address, uint32_t data);
  // }

  namespace NVS //Device should also implentment duplication check. If new value is equal to the old one, then skip the write. 
  {
    size_t Size(uint32_t hash);
    vector<char> Read(uint32_t hash);
    bool Write(uint32_t hash, void* pointer, uint16_t length);
    bool Delete(uint32_t hash);
    void Clear();
  }

#ifdef DEVICE_BATTERY
  namespace Battery
  {
    bool Chagring();
    float Voltage();
  }
#endif
}

// Matrix OS APIs available for Device Layer
namespace MatrixOS
{
  namespace Logging
  {
    void LogError(string tag, string format, ...);
    void LogWarning(string tag, string format, ...);
    void LogInfo(string tag, string format, ...);
    void LogDebug(string tag, string format, ...);
    void LogVerbose(string tag, string format, ...);
  }

  namespace SYS
  {
    void ErrorHandler(string error);
  }

  namespace KEYPAD
  {
    bool NewEvent(KeyEvent* keyevent);
  }
}