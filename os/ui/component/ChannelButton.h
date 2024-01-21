#pragma once
#include "UIComponent.h"
#include "MatrixOS.h"

class ChannelButton : public UIComponent {
 public:
  Dimension dimension;
  ChannelConfig* config;
  uint16_t count;
  bool lowBrightness;
  bool select = false;
  bool mute = false;
  bool solo = false;
  std::function<void()> callback;
  Point position = Point(0, 0);
  

  ChannelButton(Dimension dimension, ChannelConfig* config, uint16_t count, bool lowBrightness, std::function<void()> callback = nullptr) {
    this->dimension = dimension;
    this->config = config;
    this->count = count;
    this->lowBrightness = lowBrightness;
    this->callback = callback;
  }

  virtual Color GetColor() { return config->color[0]; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Callback() {
    if (callback != nullptr)
    {
      callback();
      return true;
    }
    return false;
  }

  virtual bool Render(Point origin) {
    position = origin;
    uint8_t activeChannel = MatrixOS::UserVar::global_MIDI_CH;

    for (uint8_t y = 0; y < dimension.y; y++)
    {
      for (uint8_t x = 0; x < dimension.x; x++)
      {        
        int8_t i = x + y * dimension.x;
        Point xy = origin + Point(x, y);

        if (i < count){
          if (MatrixOS::MIDI::CheckHold(SEND_CC, i, config->MuteCC)) {  
            MatrixOS::LED::SetColor(xy, COLOR_RED);
          } else if (MatrixOS::MIDI::CheckHold(SEND_CC, i, config->SoloCC)){
            MatrixOS::LED::SetColor(xy, COLOR_BLUE);
          } else if (MatrixOS::MIDI::CheckHold(SEND_CC, i, config->SelectCC)){
            MatrixOS::LED::SetColor(xy, COLOR_WHITE); 
          } else if (activeChannel == i) {
            MatrixOS::LED::SetColor(xy, config->color[i].Blink(Device::KeyPad::fnState));
          } else {
            MatrixOS::LED::SetColor(xy, config->color[i].ToLowBrightness().Blink(Device::KeyPad::fnState));
          }
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    int8_t i = xy.x + xy.y * dimension.x;

    if (keyInfo->state == PRESSED) { 
      if(Device::KeyPad::fnState == ACTIVATED || Device::KeyPad::fnState == HOLD) {
        MatrixOS::Component::Channel_Setting(config, i);
        return true;
      } else if(Device::muteState == true) {
        MatrixOS::MIDI::Hold(xy + position, SEND_CC, i, config->MuteCC, 127);
      } else if (Device::KeyPad::ShiftActived() || Device::soloState == true) {
        MatrixOS::MIDI::Hold(xy + position, SEND_CC, i, config->SelectCC, 127);
        MatrixOS::MIDI::Hold(xy + position, SEND_CC, i, config->SoloCC, 127);
        MatrixOS::UserVar::global_MIDI_CH = i;
      } else {
        MatrixOS::MIDI::Hold(xy + position, SEND_CC, i, config->SelectCC, 127);
        MatrixOS::UserVar::global_MIDI_CH = i;
      }
    } 

    if (keyInfo->state == RELEASED || keyInfo->state == IDLE) {
      
      Callback();
    }
    
    return true;
  }
};