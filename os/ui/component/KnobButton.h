#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class KnobButton : public UIComponent {
 public:
  KnobConfig* config;
  int16_t* value;
  uint8_t count;


  KnobButton(){}

  KnobButton(KnobConfig* config, int16_t* value, uint8_t count ) {
    this->config = config;
    this->value = value;
    this->count = count;
  }

  void Setup(KnobConfig* config, int16_t* value, uint8_t count ) {
    this->config = config;
    this->value = value;
    this->count = count;
  }

  virtual Color GetColor() { return config->color; }
  virtual Dimension GetSize() { return Dimension(count , 1); }

  virtual bool Render(Point origin) {

    for (uint8_t x = 0; x < count; x++)
    {
      KnobConfig* con = config + x;
      int16_t* val = value + x;
      Point xy = origin + Point(x, 0);

      if (con->enable){

        if (con->def > 62 && con->def <65)
        {
            float hue;
            float s;
            float v;
          if (*val > 64)
          {
            MatrixOS::LED::SetColor(xy, con->color.Scale((*val - 63) * 3.74 + 16));
          }
          else if (*val > 62 && *val < 65)
          {
            Color::RgbToHsv(con->color, &hue, &s, &v);
            Color color = Color::HsvToRgb(hue, 0.5, 1);
            MatrixOS::LED::SetColor(xy, color.Scale(16));
          }
          else if (*val < 63)
          {
            Color::RgbToHsv(con->color, &hue, &s, &v);
            if(0.5 - hue > 0)
              hue = 0.5 - hue;
            else
              hue = 1.5 - hue;
            Color color = Color::HsvToRgb(hue, s, v);
            MatrixOS::LED::SetColor(xy, color.Scale((63 - *val) * 3.74 + 16));
          }
        }
        else
        {
          MatrixOS::LED::SetColor(xy, con->color.Scale(*val * 1.87 + 16));
        }
      } else {
        MatrixOS::LED::SetColor(xy, COLOR_BLANK);
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    KnobConfig* con = config + xy.x;
    int16_t* val = value + xy.x;

    if (keyInfo->state == HOLD && con->enable)
    {
      *val = con->def;
      if (con->enable) MatrixOS::Component::Knob_Function(con, val);
    };

    return true;
  }
};
