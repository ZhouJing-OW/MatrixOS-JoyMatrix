#pragma once

#include "MatrixOS.h"


class MixerMatrix : public UIComponent {
 public:
  ButtonConfig* buttonConfig;
  KnobConfig* knobConfig;
  bool* active;
  uint8_t* number;
  std::function<void()> callback;

  int32_t* value;
  bool activeState[16];
  uint8_t knobMap[16];

  MixerMatrix( ButtonConfig* buttonConfig, KnobConfig* knobConfig, uint8_t* number, bool* active,  std::function<void()> callback = nullptr) {
    this->knobConfig = knobConfig;
    this->buttonConfig = buttonConfig;
    this->active = active;
    this->number = number;
    this->callback = callback;

    for (uint8_t i = 0; i < 16; i++) {

      if (i < buttonConfig->channelGroupDivide){
        knobMap[i] = i;
        activeState[i] = 0;
      } else if (i >= buttonConfig->channelGroupDivide && i < buttonConfig->channelGroupDivide + 8){
        knobMap[i] = i - buttonConfig->channelGroupDivide;
        activeState[i] = 1;
      } else {
        knobMap[i] = i - 8;
        activeState[i] = 0;
      }
    }
  }

  virtual Dimension GetSize() { return Dimension(16 , 4); }

  virtual bool Callback() {
    if (callback != nullptr)
    {
      callback();
      return true;
    }
    return false;
  }

  virtual bool Render(Point origin) {
    for (uint8_t x = 0; x < 16; x++)
    {
      for (uint8_t y = 0; y < 4; y++)
      {
        Point xy = origin + Point(x, y);
        int32_t defaultValue = knobConfig->defaultValue[3][4 + y];
        int32_t value = knobConfig->value2[3][4 + y][x];
        Color color1;

        if (activeState[x] == *active && y == *number - 4 ) 
          color1 = Color(0xFFFFFF);
        else 
          color1 = buttonConfig->channelColor[x];

        if (defaultValue > 62 && defaultValue < 65)
        {
          float hue;
          float s;
          float v;
          if (value > 64)
          {
            MatrixOS::LED::SetColor(xy, color1.Scale((value - 63) * 3.74 + 16));
          }
          else if (value > 62 && value < 65)
          {
            Color::RgbToHsv(color1, &hue, &s, &v);
            Color color2 = Color::HsvToRgb(hue, 0.5, 1);
            MatrixOS::LED::SetColor(xy, color2.Scale(16));
          }
          else if (value < 63)
          {
            Color::RgbToHsv(color1, &hue, &s, &v);
            if (0.5 - hue > 0)
              hue = 0.5 - hue;
            else
              hue = 1.5 - hue;
            Color color2 = Color::HsvToRgb(hue, s, v);
            MatrixOS::LED::SetColor(xy, color2.Scale((63 - value) * 3.74 + 16));
          }
        }
        else
        {
          MatrixOS::LED::SetColor(xy, color1.Scale(value * 1.87 + 16));
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    uint8_t ch = xy.x;
    uint8_t val1 = buttonConfig->channelSelect[xy.x];

    if (keyInfo->state == PRESSED)
    {
      if ( activeState[xy.x] == *active && xy.y == *number - 4 ){
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, ch, val1, 127));
      } 
    }

    if (keyInfo->state == HOLD)
    {
      if ( activeState[xy.x] == *active && xy.y == *number - 4){
        knobConfig->value2[3][*number][xy.x] = knobConfig->defaultValue[3][*number];
        Device::Encoder::knobFunction(knobMap[xy.x]);
      }
    }

    if (keyInfo->state == RELEASED)
    {
      if ( activeState[xy.x] == *active && xy.y == *number - 4){
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, ch, val1, 0));
        buttonConfig->activeChannel = xy.x;
      } else {
        *active = activeState[xy.x];
        *number = 4 + xy.y;

        if (Callback()){
        MLOGD("UI Fader", "Key Event Callback");
        return true;
        }        
      }
    }
    return true;
  }
};
