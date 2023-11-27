#pragma once

#include "MatrixOS.h"


class MixerFaders : public UIComponent {
 public:
  Dimension dimension;
  ButtonConfig* buttonConfig;
  KnobConfig* knobConfig;
  bool* active;
  std::function<void()> callback;

  uint8_t piece;
  bool activeState[16];
  uint8_t knobMap[16];

  MixerFaders(Dimension dimension, ButtonConfig* buttonConfig, KnobConfig* knobConfig,  bool* active, std::function<void()> callback = nullptr) {
    this->dimension = dimension;
    this->buttonConfig = buttonConfig;
    this->knobConfig = knobConfig;
    this->active = active;
    this->callback = callback;

    uint8_t n = 0;
    piece = (127 / dimension.y);

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

  virtual Dimension GetSize() { return Dimension(16, 4); }

  virtual bool Callback() {
    if (callback != nullptr)
    {
      callback();
      return true;
    }
    return false;
  }

  virtual bool Render(Point origin) {

    for (uint8_t y = 0; y < dimension.y; y++)
    {
      for (uint8_t x = 0; x < dimension.x; x++)
      {        
        Point xy = origin + Point(x, y);
        int32_t value = knobConfig->value2[3][7][x];
        Color color = buttonConfig->channelColor[x];

        if (value >= piece * (4 - y)){
          if (activeState[x] == !*active) 
            MatrixOS::LED::SetColor(xy, color.Scale(32));
          else 
            MatrixOS::LED::SetColor(xy, color);
            
        }
        else if (value + piece >= piece * (4 - y))
        {
          MatrixOS::LED::SetColor(xy, color.Scale(256 * (value % piece) / piece ));
        }
        else
        {
          MatrixOS::LED::SetColor(xy, Color(0x000000));
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
      int32_t* value = &knobConfig->value2[3][7][xy.x];
      if (activeState[xy.x] == *active) {
        *value = piece * (4 - xy.y - 0.5);
        Device::Encoder::knobFunction(knobMap[xy.x]);
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, ch, val1, 127));
      } 

    } else if (keyInfo->state == RELEASED){
      if (activeState[xy.x] == *active) {
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, ch, val1, 0));
        buttonConfig->activeChannel = xy.x;
      }
      else {
        *active = !*active;
      }

      if (Callback()){
        MLOGD("UI Fader", "Key Event Callback");
        return true;
      }
    }

    return true;
  }
};