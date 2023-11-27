#pragma once

#include "MatrixOS.h"


class MixerKnobs : public UIComponent {
 public:
  std::function<void()> callback;

  MixerKnobs(std::function<void()> callback = nullptr) {
    this->callback = callback;

  }

  virtual Dimension GetSize() { return Dimension(8 , 1); }

  virtual bool Callback() {
    if (callback != nullptr)
    {
      callback();
      return true;
    }
    return false;
  }

  virtual bool Render(Point origin) {
    for (uint8_t x = 0; x < 8; x++)
    {
      Point xy = origin + Point(x, 0);
      Color color1 = *Device::Encoder::GetColor(x);
      int32_t value = *Device::Encoder::GetValue(x);
      int32_t defaultValue = *Device::Encoder::GetDefault(x);

      if (defaultValue > 62 && defaultValue < 65)
      {
        float hue;
        float s;
        float v;
        if (value > 64)
        {
          MatrixOS::LED::SetColor(xy, color1.Scale((value - 63) * 3.74 + 16));
        }
        else if (value > 62 && value < 65)
        {
          Color::RgbToHsv(color1, &hue, &s, &v);
          Color color2 = Color::HsvToRgb(hue, 0.5, 1);
          MatrixOS::LED::SetColor(xy, color2.Scale(16));
        }
        else if (value < 63)
        {
          Color::RgbToHsv(color1, &hue, &s, &v);
          if (0.5 - hue > 0)
            hue = 0.5 - hue;
          else
            hue = 1.5 - hue;
          Color color2 = Color::HsvToRgb(hue, s, v);
          MatrixOS::LED::SetColor(xy, color2.Scale((63 - value) * 3.74 + 16));
        }
      }
      else
      {
        MatrixOS::LED::SetColor(xy, color1.Scale(value * 1.87 + 16));
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    if (keyInfo->state == HOLD)
    {
      int32_t* valuePtr = Device::Encoder::GetValue(xy.x);
      *valuePtr = *Device::Encoder::GetDefault(xy.x);
      Device::Encoder::knobFunction(xy.x);
    };
    return true;
  }
};
