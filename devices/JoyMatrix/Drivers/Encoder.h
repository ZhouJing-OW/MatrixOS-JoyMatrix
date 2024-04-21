#pragma once
#include "MatrixOS.h"
#include <queue>

class EncoderEvent {
 public:
  std::queue<uint8_t> pin;
  KnobConfig* knob = nullptr;
  int16_t* val;
  uint8_t state;
  Timer timer;

  const uint8_t highSpeed = 3;  // The encoder changes the value with each pulse in high speed mode.

  void Setup(KnobConfig* config) {
    if (config == nullptr || config->GetPtr() == nullptr) 
    { knob = nullptr; return;}
     
    this->knob = config;
    if (knob->min <= knob->max)
    {
      val = knob->GetPtr();
      if (*val < knob->min) *val = knob->min;
      if (*val > knob->max) *val = knob->max;
      if (knob->def < knob->min) knob->def = knob->min;
      if (knob->def > knob->max) knob->def = knob->max;
    }
    else
      knob = nullptr;
  }

  void push(uint8_t pin) {
    if (this->pin.size() == 0 || (this->pin.back() != pin))
      this->pin.push(pin);
  }

  bool decode() {
    if (pin.size() > 0)
    {
      uint8_t m = pin.front();
      switch (state)
      {
        case 0:  // 00 filter
          if (m != 0) 
            state = m; 
          break;
        case 1:  // 01
          if (m != 1)
          {
            if (m == 0)
            {
              state = 0;
              bool lowSpeed = pin.size() <= 8;
              ValuePlus(lowSpeed);
            }
            else
              state = m;
          }
          break;
        case 2:  // 10
          if (m != 2)
          {
            if (m == 0)
            {
              state = 0;
              bool lowSpeed = pin.size() <= 8;
              ValueMinus(lowSpeed);
            }
            else
              state = m;
          }
          break;
        case 3:  // 11
          if (m != 3)
          {
            if (m == 1)
            {
              state = 1;
              ValuePlus();
            }
            if (m == 2)
            {
              state = 2;
              ValueMinus();
            }
            if (m == 0)
            {
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

  bool Activated(uint32_t ms) { return !timer.IsLonger(ms); }

  void ValuePlus(bool lowSpeed = false) {
    bool shift = Device::KeyPad::Shift();
    bool wide = knob->max - knob->min > 48;
    int16_t temp = knob->Value() + ((shift || !wide || knob->type != SEND_CC) ? !lowSpeed : highSpeed);
    knob->SetValue(temp > knob->max ? knob->max : temp);
    MatrixOS::KnobCenter::Knob_Function(knob);
    timer.RecordCurrent();
  }

  void ValueMinus(bool lowSpeed = false) {
    bool shift = Device::KeyPad::Shift();
    bool wide = knob->max - knob->min > 48;
    int16_t temp = knob->Value() - ((shift || !wide || knob->type != SEND_CC) ? !lowSpeed : highSpeed);
    knob->SetValue(temp < knob->min ? knob->min : temp);
    MatrixOS::KnobCenter::Knob_Function(knob);
    timer.RecordCurrent();
  }

};