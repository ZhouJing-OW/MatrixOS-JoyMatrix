#pragma once
#include "UIComponent.h"
#include "MatrixOS.h"

class MidiButton : public UIComponent {
 public:
  Dimension dimension;
  MidiButtonConfig* config;
  uint16_t count;
  int8_t* active = nullptr;
  bool upward;
  bool toLowBrightness;
  Point position = Point(0, 0);

  MidiButton(Dimension dimension, MidiButtonConfig* config, uint16_t count , bool upward = false) {
    this->dimension = dimension;
    this->config = config;
    this->count = count;
    this->upward = upward;
  }

  MidiButton(Dimension dimension, MidiButtonConfig* config, uint16_t count, int8_t* active, bool upward = true, bool toLowBrightness = false) {
    this->dimension = dimension;
    this->config = config;
    this->count = count;
    this->active = active;
    this->upward = upward;
    this->toLowBrightness = toLowBrightness;
  }

  virtual Color GetColor() { return config->color; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Render(Point origin) {
    position = origin;

    for (uint8_t y = 0; y < dimension.y; y++)
    {
      for (uint8_t x = 0; x < dimension.x; x++)
      { 
        Point xy = origin + Point(x, y);
        uint8_t i;
        uint8_t shift = (Device::KeyPad::ShiftActived() && count > dimension.Area()) * dimension.Area();
        if (upward) i = (dimension.y - y - 1)* dimension.x + x + shift;
        else i = y * dimension.x + x + shift;
        MidiButtonConfig* con = config + i;
        int8_t channel = con->globalChannel ? MatrixOS::UserVar::global_MIDI_CH : con->channel;

        if (i < count){
          if (MatrixOS::MIDI::CheckHold(con->type, channel, con->byte1)){
            MatrixOS::LED::SetColor(xy, COLOR_WHITE);
          } else if ((active != nullptr) && toLowBrightness && (i != *active)) {
            MatrixOS::LED::SetColor(xy, con->color.ToLowBrightness().Blink(Device::KeyPad::fnState));
          } else {
            MatrixOS::LED::SetColor(xy, con->color.Blink(Device::KeyPad::fnState));
          }
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    uint8_t i;
    uint8_t shift = (Device::KeyPad::ShiftActived() && count > dimension.Area()) * dimension.Area();
    if (upward) i = (dimension.y - xy.y - 1)* dimension.x + xy.x + shift;
    else i = xy.y * dimension.x + xy.x + shift;
    MidiButtonConfig* con = config + i;
    int8_t channel = con->globalChannel ? MatrixOS::UserVar::global_MIDI_CH : con->channel;

    if(i < count){
      if (keyInfo->state == PRESSED) {
        if((Device::KeyPad::fnState == ACTIVATED || Device::KeyPad::fnState == HOLD)) {
          MatrixOS::Component::Button_Setting(con);
          return true;
        } 
        if (active!= nullptr) *active = i;
        MatrixOS::MIDI::Hold(xy + position, con->type, channel, con->byte1, con->byte2);
        return true;
      } 
    }
    return false;
  }
};