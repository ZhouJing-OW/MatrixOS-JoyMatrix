#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class ChannelButton : public UIComponent {
 public:
  Dimension dimension;
  ChannelButtonConfig* config;
  uint16_t count;
  bool lowBrightness;
  

  ChannelButton(Dimension dimension,ChannelButtonConfig* config, uint16_t count, bool lowBrightness) {
    this->dimension = dimension;
    this->config = config;
    this->count = count;
    this->lowBrightness = lowBrightness;
  }

  virtual Color GetColor() { return config->color; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Render(Point origin) {

    uint8_t activeChannel = MatrixOS::UserVar::global_MIDI_CH;

    for (uint8_t y = 0; y < dimension.y; y++)
    {
      for (uint8_t x = 0; x < dimension.x; x++)
      {        
        int8_t i = x + y * dimension.x;
        Point xy = origin + Point(x, y);
        if (i < count){
          if ((config + i)->pressed) { // If find the Button is currently active. Show it as white
            if (Device::KeyPad::LShiftState == ACTIVATED || Device::muteState == true){
              MatrixOS::LED::SetColor(xy, COLOR_RED);
            } else if (Device::KeyPad::RShiftState == ACTIVATED || Device::soloState == true){
              MatrixOS::LED::SetColor(xy, COLOR_BLUE);
            } else {
            MatrixOS::LED::SetColor(xy, COLOR_WHITE); 
            }
          } else if (activeChannel == i) {
            MatrixOS::LED::SetColor(xy, (config + i)->color);
          } else {
            MatrixOS::LED::SetColor(xy, (config + i)->color.ToLowBrightness());
          }
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    int8_t i = xy.x + xy.y * dimension.x;

    if (keyInfo->state == PRESSED)  
    { 
      
      if(Device::KeyPad::LShiftState == ACTIVATED || Device::muteState == true){
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, (config + i)->channel, (config + i)->channelMuteCC, 127));
      }
      else if (Device::KeyPad::RShiftState == ACTIVATED || Device::soloState == true)
      {
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, (config + i)->channel, (config + i)->channelSelectCC, 127));
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, (config + i)->channel, (config + i)->channelSoloCC, 127));
        MatrixOS::UserVar::global_MIDI_CH = (config + i)->channel;
      }
      else
      {
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, (config + i)->channel, (config + i)->channelSelectCC, 127));
        MatrixOS::UserVar::global_MIDI_CH = (config + i)->channel;
      }
      (config + i)->pressed = true;
    } if (keyInfo->state == CLEARED) {(config + i)->pressed = false;}

    return true;
  }
};