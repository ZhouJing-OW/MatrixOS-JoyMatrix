#pragma once

#include "MatrixOS.h"


class ChannelButton : public UIComponent {
 public:
  Dimension dimension;
  uint8_t scale;
  uint8_t activeChannel;
  ButtonConfig* config;
  std::function<void()> callback;
  std::unordered_map<uint8_t, uint8_t> activeCH;
  

  ChannelButton(Dimension dimension, uint8_t scale, ButtonConfig* config, std::function<void()> callback = nullptr) {
    this->dimension = dimension;
    this->scale = scale;
    this->config = config;
    this->callback = callback;
  }

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

    activeChannel = config->activeChannel;

    for (uint8_t y = 0; y < dimension.y; y++)
    {
      for (uint8_t x = 0; x < dimension.x; x++)
      {        
        int8_t i = x + y * dimension.x;
        Point xy = origin + Point(x, y);

        if (activeCH.find(i) != activeCH.end())  // If find the Button is currently active. Show it as white
        { MatrixOS::LED::SetColor(xy, Device::muteState ? Color(0xFF0000) : Color(0xFFFFFF)); }
        else {
          MatrixOS::LED::SetColor(xy, config->channelColor[i].Scale( (activeChannel == i) ? 255 : scale));
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    int8_t i = xy.x + xy.y * dimension.x;

    if (keyInfo->state == PRESSED)  
    { 
      if(Device::muteState == true){
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channelCH[i], config->channelMute[i], 127));
      }
      else if (Device::leftShift | Device::rightShift)
      {
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channelCH[i], config->channelSelect[i], 127));
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channelCH[i], config->channelSolo[i], 127));
        activeChannel = i;
        config->activeChannel = i;
      }
      else
      {
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channelCH[i], config->channelSelect[i], 127));
        activeChannel = i;
        config->activeChannel = i;
      }

      activeCH[i]++;

      if (Callback()){
        MLOGD("UI Button", "Key Event Callback");
        return true;
      }
    }
    else if (keyInfo->state == RELEASED)
    {
      if(Device::muteState == true){
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channelCH[i], config->channelMute[i], 0));
      } else if(Device::leftShift | Device::rightShift){
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channelCH[i], config->channelSelect[i], 0));
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channelCH[i], config->channelSolo[i], 0));
      } else {
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channelCH[i], config->channelSelect[i], 0));
      }

      if (activeCH[i]-- <= 1){ activeCH.erase(i);}

    };

    return true;
  }
};