#pragma once

#include "MatrixOS.h"


class MixerFader : public UIComponent {
 public:
  Dimension dimension;
  KnobConfig* config;
  uint8_t count;
  bool active[16];

  MixerFader(Dimension dimension, KnobConfig* config, uint8_t count) {
    this->dimension = dimension;
    this->config = config;
    this->count = count;
  }

  uint8_t piece = (127 / dimension.y);

  virtual Color GetColor() { return config->color; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Render(Point origin) {

    for (uint8_t y = 0; y < dimension.y; y++){ 
      for (uint8_t x = 0; x < dimension.x; x++){
        Point xy = origin + Point(x, y);

        if(x < count){        
          int32_t value = (config + x)->value2;
          Color color = (config + x)->color;

          if (value >= piece * (dimension.y - y)){
            if (active[x] == !*active) 
              MatrixOS::LED::SetColor(xy, color.ToLowBrightness());
            else 
              MatrixOS::LED::SetColor(xy, color);
          } else if (value + piece >= piece * (dimension.y - y)) {
            MatrixOS::LED::SetColor(xy, color.Scale(256 * (value % piece) / piece ));
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

    if (keyInfo->state == PRESSED)  
    { 
      if (active[xy.x] == true) {
        (config + xy.x)->value2 = piece * (dimension.y - xy.y - 0.5);
        if (Device::KeyPad::ShiftActived()) {
          (config + xy.x)->shiftCallback();
        } else {
          (config + xy.x)->callback();
        }
      } 

    } else if (keyInfo->state == RELEASED) {
      if (active[xy.x] != true) {
        bool c = xy.x > 7;
        for (uint8_t n = 0; n < 8; n++){
          active[c * 8 + n] = true;
          active[!c * 8 + n] = false;
        }
      }
    }

    return true;
  }
};