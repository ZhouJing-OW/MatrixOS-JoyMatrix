#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class KnobButton : public UIComponent {
 public:
  KnobConfig* config;
  uint8_t count;


  KnobButton(){}

  KnobButton(KnobConfig* config, uint8_t count ) {
    this->config = config;
    this->count = count;
  }

  void Setup(KnobConfig* config, uint8_t count ) {
    this->config = config;
    this->count = count;
  }

  virtual Color GetColor() { return config->color; }
  virtual Dimension GetSize() { return Dimension(count , 1); }

  virtual bool Render(Point origin) {

    for (uint8_t x = 0; x < count; x++)
    {        
      int8_t i = x;
      Point xy = origin + Point(x, 0);

      if ((config + i)->enable){
        int32_t value = (config + i)->value2;

        if ((config + i)->def > 62 && (config + i)->def <65)
        {
            float hue;
            float s;
            float v;
          if (value > 64)
          {
            MatrixOS::LED::SetColor(xy, (config + i)->color.Scale((value - 63) * 3.74 + 16));
          }
          else if (value > 62 && value < 65)
          {
            Color::RgbToHsv((config + i)->color, &hue, &s, &v);
            Color color = Color::HsvToRgb(hue, 0.5, 1);
            MatrixOS::LED::SetColor(xy, color.Scale(16));
          }
          else if (value < 63)
          {
            Color::RgbToHsv((config + i)->color, &hue, &s, &v);
            if(0.5 - hue > 0)
              hue = 0.5 - hue;
            else
              hue = 1.5 - hue;
            Color color = Color::HsvToRgb(hue, s, v);
            MatrixOS::LED::SetColor(xy, color.Scale((63 - value) * 3.74 + 16));
          }
        }
        else
        {
          MatrixOS::LED::SetColor(xy, (config + i)->color.Scale(value * 1.87 + 16));
        }
      } else {
        MatrixOS::LED::SetColor(xy, COLOR_BLANK);
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    int8_t i = xy.x;

    if (keyInfo->state == HOLD && (config + i)->enable)
    {
      (config + i)->value2 = (config + i)->def;
      if(Device::KeyPad::ShiftActived()){
        (config + i)->shiftCallback();
      } else {
        (config + i)->callback();
      }
    };

    return true;
  }
};
