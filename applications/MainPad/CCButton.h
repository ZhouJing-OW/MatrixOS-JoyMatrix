#pragma once

#include "MatrixOS.h"


class CCButton : public UIComponent {
 public:
  Dimension dimension;
  uint8_t scale;
  uint8_t activeButton;
  ButtonConfig* config;
  uint8_t* group;
  std::function<void()> callback;
  std::unordered_map<uint8_t, uint8_t> activeCC;
  std::unordered_map<uint8_t, uint8_t> activeNote;
  std::unordered_map<uint8_t, uint8_t> activeChannelChange;
  

  CCButton(Dimension dimension, uint8_t scale, ButtonConfig* config, uint8_t* group , std::function<void()> callback = nullptr) {
    this->dimension = dimension;
    this->scale = scale;
    this->config = config;
    this->group = group;
    this->callback = callback;
  }

  virtual Color GetColor() { return config->color[*group]; }
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

    activeButton = config->activeButton[*group];

    for (uint8_t y = 0; y < dimension.y; y++)
    {
      for (uint8_t x = 0; x < dimension.x; x++)
      {        
        int8_t i = x + (dimension.y - 1 - y) * dimension.x;
        Point xy = origin + Point(x, y);

        if (activeCC.find(i) != activeCC.end())  // If find the Button is currently active. Show it as white
        { MatrixOS::LED::SetColor(xy, Color(0xFFFFFF)); }
        else {
          MatrixOS::LED::SetColor(xy, config->color[*group].Scale( (activeButton == i) ? 255 : scale));
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    int8_t i = xy.x + (dimension.y - 1 - xy.y) * dimension.x;

    uint8_t pressure;
    if (Device::pressure > MatrixOS::UserVar::pressureToVelocity_Min)
      pressure = Device::pressure;
    else
      pressure = MatrixOS::UserVar::pressureToVelocity_Min;
    if (pressure > MatrixOS::UserVar::pressureToVelocity_Max)
      pressure = MatrixOS::UserVar::pressureToVelocity_Max;

    if (keyInfo->state == PRESSED)  
    { 
      switch (config->type[*group][i]){
        case 0: // Command Change
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channel[*group][i], config->value1[*group][i], config->value2[*group][i]));
          break;
        case 1: // Program Change
          MatrixOS::MIDI::Send(MidiPacket(0, ProgramChange, config->channel[*group][i], config->value1[*group][i], 0));
          break;
        case 2: // Note On
          MatrixOS::MIDI::Send(MidiPacket(0, NoteOn, config->channel[*group][i], config->value1[*group][i], MatrixOS::UserVar::pressureSensitive ? pressure : config->value2[*group][i]));
          activeNote[i]++;
          if (Device::leftShift || Device::rightShift){
            MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channel[*group][i], config->channelSelect[config->channel[*group][i]], 127));
            config->activeChannel = config->channel[*group][i];
            activeChannelChange[i]++;
          }
      };

      activeButton = i;
      config->activeButton[*group] = i;
      activeCC[i]++;

      if (Callback()){
        MLOGD("UI Button", "Key Event Callback");
        return true;
      }
    }
    else if (keyInfo->state == RELEASED)
    {

      switch (config->type[*group][i]){
        case 0: // Command Change
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channel[*group][i], config->value1[*group][i], 0));
          break;
        case 1: // Program Change
          break;
        case 2: // Note Off
          if(activeNote[i]){
            MatrixOS::MIDI::Send(MidiPacket(0, NoteOn, config->channel[*group][i], config->value1[*group][i], 0));
            if (activeNote[i]-- <= 1){ activeCC.erase(i);}
          }
          if(activeChannelChange[i]){
            MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channel[*group][i], config->channelSelect[config->channel[*group][i]], 0));
            if (activeChannelChange[i]-- <= 1){ activeCC.erase(i);}
          }
      };

      if (activeCC[i]-- <= 1){ activeCC.erase(i);}

    };

    return true;
  }
};