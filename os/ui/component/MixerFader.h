#pragma once
#include "MatrixOS.h"

class MixerFader : public UIComponent {
 public:
  Dimension dimension;
  KnobConfig* config;
  int16_t* value;
  uint8_t count;
  bool active[16];

  MixerFader(Dimension dimension, KnobConfig* config, int16_t* value, uint8_t count) {
    this->dimension = dimension;
    this->config = config;
    this->value = value;
    this->count = count;
  }

  uint8_t piece = (127 / dimension.y);

  virtual Color GetColor() { return config->color; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Render(Point origin) {

    for (uint8_t y = 0; y < dimension.y; y++){ 
      for (uint8_t x = 0; x < dimension.x; x++){
        Point xy = origin + Point(x, y);
        KnobConfig* con = config + x;
        
        if(x < count){        
          int16_t* val = value + x;
          Color color = con->color;

          if (*val >= piece * (dimension.y - y)){
            if (active[x] == !*active) 
              MatrixOS::LED::SetColor(xy, color.ToLowBrightness());
            else 
              MatrixOS::LED::SetColor(xy, color);
          } else if (*val + piece >= piece * (dimension.y - y)) {
            MatrixOS::LED::SetColor(xy, color.Scale(256 * (*val % piece) / piece ));
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
    KnobConfig* con = config + xy.x;
    int16_t* val = value + xy.x;
    if (keyInfo->state == PRESSED)  
    { 
      if (active[xy.x] == true) {
        con->byte2 = piece * (dimension.y - xy.y - 0.5);
        MatrixOS::Component::Knob_Function(con, val);
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