#pragma once
#include "UIComponent.h"
#include "MatrixOS.h"

class MidiButton : public UIComponent {
 public:
  Dimension dimension;
  MidiButtonConfig* config;
  uint16_t count;
  int8_t* activePoint = nullptr;
  bool upward;
  bool ToLowBrightness;
  Point position = Point(0, 0);

  MidiButton(Dimension dimension, MidiButtonConfig* config, uint16_t count, bool upward = false, int8_t* activePoint = nullptr , bool ToLowBrightness = false) {
    this->dimension = dimension;
    this->config = config;
    this->count = count;
    this->activePoint = activePoint;
    this->upward = upward;
    this->ToLowBrightness = ToLowBrightness;
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
        uint8_t shift = (Device::KeyPad::Shift() && count > dimension.Area()) * dimension.Area();
        if (upward) 
          i = (dimension.y - y - 1)* dimension.x + x + shift;
        else 
          i = y * dimension.x + x + shift;
        MidiButtonConfig* con = config + i;
        int8_t channel = con->globalChannel ? MatrixOS::UserVar::global_channel : con->channel;

        if (i < count)
        {
          if (MatrixOS::MidiCenter::FindHold(con->type, channel, con->byte1))
          {
            MatrixOS::LED::SetColor(xy, COLOR_WHITE);
          } 
          else if (activePoint != nullptr && ToLowBrightness == true && i != *activePoint) 
          {
            Color thisColor = con->color.ToLowBrightness();
            MatrixOS::LED::SetColor(xy, thisColor.Blink_Key(Device::KeyPad::fnState));
          } 
          else 
          {
            MatrixOS::LED::SetColor(xy, con->color.Blink_Key(Device::KeyPad::fnState));
          }
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    uint8_t i;
    uint8_t shift = (Device::KeyPad::Shift() && count > dimension.Area()) * dimension.Area();
    if (upward) i = (dimension.y - xy.y - 1)* dimension.x + xy.x + shift;
    else i = xy.y * dimension.x + xy.x + shift;
    MidiButtonConfig* con = config + i;
    int8_t channel = con->globalChannel ? MatrixOS::UserVar::global_channel : con->channel;

    if(i < count){
      if (keyInfo->state == PRESSED) 
      {
        if(Device::KeyPad::fnState.active()) 
        {
          MatrixOS::Component::Button_Setting(config, i);
          return true;
        } 
        if (activePoint!= nullptr) 
          *activePoint = i;
        if (con->toggleMode == true) 
          MatrixOS::MidiCenter::Toggle(con->type, channel, con->byte1, con->byte2);
        else 
          MatrixOS::MidiCenter::Hold(xy + position, con->type, channel, con->byte1, con->byte2);
        return true;
      } 
    }
    return false;
  }
};