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
    uint16_t PCF8575_Read();
    void ADS1115_Write(uint8_t ADDR, uint8_t Reg, uint8_t reg_MSB, uint8_t reg_LSB);
    uint16_t ADS1115_Read(uint8_t ADDR);
    void ADS1115_Setting(uint8_t ADDR, uint8_t MUX);
  }

  namespace Rocker
  {
    void Init();
    void Scan();
    int8_t GetX(uint8_t LR);
    int8_t GetY(uint8_t LR);
    uint8_t GetButton(uint8_t LR);
    uint8_t GetPressure(uint8_t LR);
    void SetChannel(uint8_t* channel);
    void ModWheelOff(uint8_t n);
    void ModWheelLock();
  }

  namespace Encoder
  {
    void Init();
    void Setup(Color* ColorInput, uint8_t Type , uint8_t Ch , uint8_t Val , int32_t* Ptr, int32_t Min ,int32_t Max, int32_t Default, uint8_t n);
    void Scan();
    void Encoder_Read(void* arg);
    void encoder_isr_handler(void* arg);
    Color* GetColor(uint8_t n);
    int32_t* GetValue(uint8_t n);
    int32_t* GetDefault(uint8_t n);
    uint8_t* GetActiveKnob();
    void knobFunction(uint8_t n);
  }

  namespace NVS
  {
    void Init();
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

