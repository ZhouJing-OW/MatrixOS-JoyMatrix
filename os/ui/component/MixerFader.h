#pragma once
#include "MatrixOS.h"
#include "KnobButton.h"

class MixerFader : public KnobButton {
 public:

  bool vertical;
  uint8_t active = 0;

  MixerFader(){};
  MixerFader(Dimension dimension, uint8_t count, bool vertical = true) {
    KnobButton(dimension, count);
    this->vertical = vertical;
  };

  virtual bool Render(Point origin) {

    for (uint8_t y = 0; y < dimension.y; y++){ 
      for (uint8_t x = 0; x < dimension.x; x++){
        Point xy = origin + Point(x, y);
        uint8_t n;
        if (vertical) n = x; else n = y;
        
        if(n < count){
          uint8_t piece = ((knob[n].max - knob[n].min) / vertical ? dimension.y : dimension.x);        
          if (knob[n].byte2 >= piece * (dimension.y - y)){
            if (n >= active && n < active + 8) 
              MatrixOS::LED::SetColor(xy, knob[n].color.ToLowBrightness());
            else 
              MatrixOS::LED::SetColor(xy, knob[n].color);
          } else if (knob[n].byte2 + piece >= piece * (dimension.y - y)) {
            MatrixOS::LED::SetColor(xy, knob[n].color.Scale(256 * (knob[n].byte2 % piece) / piece ));
          } else {
            MatrixOS::LED::SetColor(xy, COLOR_BLANK);
          }
        } else {
        MatrixOS::LED::SetColor(xy, COLOR_BLANK);
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    uint8_t n;
    bool inActiveArea = (n >= active && n < active + 8);
    if(vertical) n = xy.x; else n = xy.y;
    if(n < count){
      if (keyInfo->state == PRESSED)  
      { 
        if (inActiveArea) {
          uint8_t piece = ((knob[n].max - knob[n].min) / vertical ? dimension.y : dimension.x);
          knob[n].byte2 = piece * (dimension.y - xy.y - 0.5);
          MatrixOS::Component::Knob_Function(&knob[n]);
        }
      } else if (keyInfo->state == RELEASED) {
        if (!inActiveArea) {
          active = n - n % 8;
        }
      }
      return true;
    }
    return false;
  }
};