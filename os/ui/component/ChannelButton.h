#pragma once
#include "UIComponent.h"
#include "MatrixOS.h"

class ChannelButton : public UIComponent {
 public:
  Dimension dimension;
  ChannelConfig* config;
  TransportState* state;
  bool lowBrightness;
  bool select = false;
  std::function<void()> callback;
  Point position = Point(0, 0);
  
  ChannelButton(Dimension dimension, ChannelConfig* config, TransportState* state, bool lowBrightness, std::function<void()> callback = nullptr) {
    this->dimension = dimension;
    this->config = config;
    this->state = state;
    this->lowBrightness = lowBrightness;
    this->callback = callback;
  }

  virtual Color GetColor() { return config->color[0]; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Callback() 
  {
    if (callback != nullptr) 
    {
      callback();
      return true;
    }
    return false;
  }

  virtual bool Render(Point origin) 
  {
    position = origin;
    if(Device::KeyPad::Shift()) state->solo = true; else state->solo = false;

    for (uint8_t y = 0; y < dimension.y; y++)
    {
      for (uint8_t x = 0; x < dimension.x; x++)
      {        
        int8_t i = x + y * dimension.x;
        Point xy = origin + Point(x, y);
        if (i < 16)
        {
          Color color;
          Color blinkColor = COLOR_WHITE;
          if (state->solo) color = config->color[i].Blink_Color(config->channelSolo[i], COLOR_BLUE);
          else if (state->mute) color = config->color[i].Blink_Color(config->channelMute[i], COLOR_RED);
          else color = config->color[i].Blink_Key(Device::KeyPad::fnState);

          if (MatrixOS::MidiCenter::FindHold(SEND_CC, i, config->muteCC)) 
              MatrixOS::LED::SetColor(xy, COLOR_RED);
          else if (MatrixOS::MidiCenter::FindHold(SEND_CC, i, config->soloCC))
              MatrixOS::LED::SetColor(xy, COLOR_BLUE);
          else if (MatrixOS::MidiCenter::FindHold(SEND_CC, i, config->selectCC))
              MatrixOS::LED::SetColor(xy, COLOR_WHITE); 
          else if (i == MatrixOS::UserVar::global_channel) 
              MatrixOS::LED::SetColor(xy, blinkColor.Blink_Color(true, color));
          else 
              MatrixOS::LED::SetColor(xy, color);
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    int8_t i = xy.x + xy.y * dimension.x;
    if (i < 16){
      if (keyInfo->state == PRESSED) { //setting
        if(Device::KeyPad::fnState.active()) 
        {
          if(!(state->mute == true || state->solo == true))
          {
            MatrixOS::Component::Channel_Setting(config, i);
            return true;
          }
        } 

        if (state->solo == true) 
        { //solo
          config->channelSolo[i] = !config->channelSolo[i];
          MatrixOS::MidiCenter::Hold(xy + position, SEND_CC, i, config->soloCC, 127);

          if (i != MatrixOS::UserVar::global_channel )
          {
            MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, i, config->selectCC, 127));
            MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, i, config->selectCC, 0));
            MatrixOS::UserVar::global_channel = i;
          }
        } 
        else if(state->mute == true) //mute
        { 
          config->channelMute[i] = !config->channelMute[i];
          MatrixOS::MidiCenter::Hold(xy + position, SEND_CC, i, config->muteCC, 127);
        } 
        else // default
        { 
          MatrixOS::MidiCenter::Hold(xy + position, SEND_CC, i, config->selectCC, 127);
          if (i != MatrixOS::UserVar::global_channel )
            MatrixOS::UserVar::global_channel = i;
        }
        return true;
      }

      if (keyInfo->state == RELEASED) 
      {
        Callback();
      }
      return true;
    } 
    return false;
  }
};