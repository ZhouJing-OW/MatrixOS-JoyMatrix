#pragma once

#include "MatrixOS.h"


class Knob : public UIComponent {
 public:
  Color* color;
  uint8_t knob;
  int32_t* defaultValue;
  std::function<void()> callback;

  int32_t* value;

  Knob(Color* color, uint8_t knob, int32_t* defaultValue, std::function<void()> callback = nullptr) {
    this->color = color;
    this->knob = knob;
    this->defaultValue = defaultValue;
    this->callback = callback;

    value = Device::Encoder::GetValue(knob);
  }

  virtual Color GetColor() { return *color; }
  virtual Dimension GetSize() { return Dimension(1 , 1); }

  virtual bool Callback() {
    if (callback != nullptr)
    {
      callback();
      return true;
    }
    return false;
  }

  virtual bool Render(Point origin) {

    Point xy = origin;
    if (*defaultValue > 62 && *defaultValue < 65)
    {
      float hue;
      float s;
      float v;
      if (*value > 64)
      {
        Color color1 = *color;
        MatrixOS::LED::SetColor(xy, color1.Scale((*value - 63) * 3.74 + 16));
      }
      else if (*value > 62 && *value < 65)
      {
        Color::RgbToHsv(*color, &hue, &s, &v);
        Color color2 = Color::HsvToRgb(hue, 0.5, 1);
        MatrixOS::LED::SetColor(xy, color2.Scale(16));
      }
      else if (*value < 63)
      {
        Color::RgbToHsv(*color, &hue, &s, &v);
        if (0.5 - hue > 0)
          hue = 0.5 - hue;
        else
          hue = 1.5 - hue;
        Color color2 = Color::HsvToRgb(hue, s, v);
        MatrixOS::LED::SetColor(xy, color2.Scale((63 - *value) * 3.74 + 16));
      }
    }
    else
    {
      Color color1 = *color;
      MatrixOS::LED::SetColor(xy, color1.Scale(*value * 1.87 + 16));
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    if (keyInfo->state == HOLD)
    {
      *value = *defaultValue;
      Device::Encoder::knobFunction(knob);
    };
    return true;
  }
};
