// Declear Family specific function
#pragma once

#include "hal/gpio_ll.h"
#include "hal/usb_hal.h"
#include "soc/usb_periph.h"

#include "driver/rmt.h"

#include "esp_task_wdt.h"
#include "esp_rom_gpio.h"
#include "esp_log.h"
#include "esp_efuse.h"
#include "esp_efuse_table.h"
#include "esp_adc/adc_oneshot.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#include "nvs_flash.h"

#include "esp_private/system_internal.h"

#include "WS2812/WS2812.h"
#include "framework/Color.h"


#define FUNCTION_KEY 0  // Keypad Code for main function key
#define RSHIFT_KEY   1
#define LSHIFT_KEY   2
#define LALT_KEY     3
#define RALT_KEY     4
#define DEVICE_SETTING

#define DEVICE_SAVED_VAR_SCOPE "Device"

namespace Device
{
  // Device Variable
  // inline CreateSavedVar(DEVICE_SAVED_VAR_SCOPE, touchbar_enable, bool, false);
  inline CreateSavedVar(DEVICE_SAVED_VAR_SCOPE, bluetooth, bool, false);

  void LoadDeviceInfo();
  void LoadVarientInfo();

  namespace USB
  {
    void Init();
  }
  namespace LED
  {
    void Init();
    void Start();
  }

  namespace KeyPad
  {
    void Init();
    void InitFN();
    void InitKeyPad();
    // void InitTouchBar();

    void Start();
    void StartKeyPad();
    // void StartTouchBar();

    // If return true, meaning the scan in intrupted
    void Scan();
    bool ScanKeyPad();
    bool ScanFN();
    bool ShiftActived();
    bool AltActived();
    uint8_t GetVelocity();
    // bool ScanTouchBar();

    namespace Binary
    {
      void Init();
      void Start();
      bool Scan();
    }

    namespace FSR
    {
      void Init();
      void Start();
      bool Scan();
    }

    bool NotifyOS(uint16_t keyID, KeyInfo* keyInfo);  // Passthough MatrixOS::KeyPad::NewEvent() result

  }

  namespace I2C
  {
    void Init();
    uint16_t PCF8574_Read();
  }

  namespace AnalogInput
  {
    void Init();
    void Scan();
    uint16_t* GetPtr(string input);
    uint16_t GetRaw(string input);
    int8_t GetRocker(AnalogConfig config);
    void PitchWheel();
  }

  namespace Encoder
  {
    void Init();
    bool Setup(KnobConfig *config, uint8_t n);
    bool Disable(uint8_t n);
    void DisableAll();
    KnobConfig* GetConfig(uint8_t n);
  }

  namespace NVS
  {
    void Init();
  }

  namespace FATFS
  {
    void Init();
    void Format();
  }

  namespace WIFI
  {
    void Init();
  }

  namespace BLEMIDI
  {
    extern bool started;
    void Init(string name);
    void Toggle();
    void Start();
    void Stop();
    bool SendMidi(uint8_t* packet);
    uint32_t MidiAvailable();
    MidiPacket GetMidi();
  }

  namespace HWMidi
  {
    void Init();
  }

  namespace ESPNOW
  {
    extern bool started;
    void Init();
    void Flush(void* param);
    bool SendMidi(uint8_t* packet);
    uint32_t MidiAvailable();
    MidiPacket GetMidi();
    void BroadcastMac();
    void UpdatePeer(const uint8_t* new_mac);
  }

  
}

