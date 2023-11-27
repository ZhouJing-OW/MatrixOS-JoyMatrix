#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"


class MidiButton : public UIComponent {
 public:
  Dimension dimension;
  MidiButtonConfig* config;
  uint16_t count;
  uint8_t* activeButton;
  bool lowBrightness;
  Point position = Point(0, 0);

  MidiButton(Dimension dimension, MidiButtonConfig* config, uint16_t count, uint8_t* activeButtonm, bool lowBrightness) {
    this->dimension = dimension;
    this->config = config;
    this->count = count;
    this->activeButton = activeButton;
    this->lowBrightness = lowBrightness;
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
        uint8_t i = y * dimension.x + x;

        if (i < count){
          if ((config + i)->pressed == true){
            MatrixOS::LED::SetColor(xy, COLOR_BLANK);
          }else if ((i != *activeButton) || lowBrightness) {
            MatrixOS::LED::SetColor(xy, (config + i)->color.ToLowBrightness());
          } else {
            MatrixOS::LED::SetColor(xy, (config + i)->color);
          }
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    int8_t i = xy.y * dimension.x + xy.x;
    if(i < count){
      if (keyInfo->state == PRESSED)  
      { 
        *activeButton = i;
        (config + i)->pressed = true;

        switch ((config + i)->type){
          case 0: // Command Change
            MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, (config + i)->channel, (config + i)->value1, (config + i)->value2));
            break;
          case 1: // Program Change
            MatrixOS::MIDI::Send(MidiPacket(0, ProgramChange, (config + i)->channel, (config + i)->value1, (config + i)->value2));
            break;
          case 2: // Note On
            MatrixOS::MIDI::HoldNote((config + i)->channel, (config + i)->value1, xy + position);
            break;
        }
        return true;

      } else if (keyInfo->state == CLEARED){
        (config + i)->pressed = false;
        return false;
      }
    }
    return false;
  }
};