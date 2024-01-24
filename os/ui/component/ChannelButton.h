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
  bool toggleMute[16];
  bool toggleSolo[16];
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

  virtual bool Callback() {
    if (callback != nullptr) {
      callback();
      return true;
    }
    return false;
  }

  virtual bool Render(Point origin) {
    position = origin;
    if(Device::KeyPad::ShiftActived()) state->solo = true; else state->solo = false;

    for (uint8_t y = 0; y < dimension.y; y++)
    {
      for (uint8_t x = 0; x < dimension.x; x++)
      {        
        int8_t i = x + y * dimension.x;
        Point xy = origin + Point(x, y);

        if (i < 16){
          Color color;
          if (state->solo) color = config->color[i].Blink(state->channelSolo[i], COLOR_BLUE);
          else if (state->mute) color = config->color[i].Blink(state->channelMute[i], COLOR_RED);
          else color = config->color[i].Blink(Device::KeyPad::fnState);

          if (MatrixOS::MIDI::CheckHold(SEND_CC, i, config->muteCC)) {  
              MatrixOS::LED::SetColor(xy, COLOR_RED);
          } else if (MatrixOS::MIDI::CheckHold(SEND_CC, i, config->soloCC)){
              MatrixOS::LED::SetColor(xy, COLOR_BLUE);
          } else if (MatrixOS::MIDI::CheckHold(SEND_CC, i, config->selectCC)){
              MatrixOS::LED::SetColor(xy, COLOR_WHITE); 
          } else if (i == MatrixOS::UserVar::global_MIDI_CH) {
              MatrixOS::LED::SetColor(xy, color);
          } else {
              MatrixOS::LED::SetColor(xy, color.ToLowBrightness());
          }
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    int8_t i = xy.x + xy.y * dimension.x;
    if (i < 16){
      if (keyInfo->state == PRESSED) { //setting
        if(Device::KeyPad::fnState == ACTIVATED || Device::KeyPad::fnState == HOLD) {
          if(!(state->mute || state->solo)){
            MatrixOS::Component::Channel_Setting(config, i);
            return true;
          }
        } 
        
        if (state->solo) { //solo
          if(!toggleSolo[i]){
            state->channelSolo[i] = !state->channelSolo[i];
            toggleSolo[i] = true;
          }
          
          MatrixOS::MIDI::Hold(xy + position, SEND_CC, i, config->soloCC, 127);
          if (i != MatrixOS::UserVar::global_MIDI_CH ){
            MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, i, config->selectCC, 127));
            MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, i, config->selectCC, 0));
            MatrixOS::UserVar::global_MIDI_CH = i;
          }
        } else if(state->mute) { //mute
          if(!toggleMute[i]){
            state->channelMute[i] = !state->channelMute[i];
            toggleMute[i] = true;
          }
          
          MatrixOS::MIDI::Hold(xy + position, SEND_CC, i, config->muteCC, 127);
        } else { // default
          MatrixOS::MIDI::Hold(xy + position, SEND_CC, i, config->selectCC, 127);
          if (i != MatrixOS::UserVar::global_MIDI_CH ){
            MatrixOS::UserVar::global_MIDI_CH = i;
          } else {
            config->type[i] += 1;
            if(config->type[i] > 2) config->type[i] = 0;
          }
        }
        return true;
      }
    } 

    if (keyInfo->state == RELEASED || keyInfo->state == IDLE) {
      toggleMute[i] = false;
      toggleSolo[i] = false;
      Callback();
    }
    
    return true;
  }
};