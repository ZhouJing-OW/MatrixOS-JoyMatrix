#pragma once

#include "MatrixOS.h"


class Knobs : public UIComponent {
 public:
  Dimension dimension;
  KnobConfig* config;
  uint8_t activeKnob;
  uint8_t* group;
  std::function<void()> callback;

  Knobs(Dimension dimension, KnobConfig* config, uint8_t* group, std::function<void()> callback = nullptr) {
    this->dimension = dimension;
    this->config = config;
    this->group = group;
    this->callback = callback;
  }

  virtual Color GetColor() { return config->color[*group]; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Callback() {
    if (callback != nullptr)
    {
      callback();
      return true;
    }
    return false;
  }

  virtual bool Render(Point origin) {

    for (uint8_t y = 0; y < dimension.y; y++)
    {
      for (uint8_t x = 0; x < dimension.x; x++)
      {        
        int8_t i = x + y * dimension.x;
        Point xy = origin + Point(x, y);
        int32_t value = *Device::Encoder::GetValue(i);
        int32_t defaultValue = config->defaultValue[*group][i];
        if (defaultValue > 62 && defaultValue <65)
        {
            float hue;
            float s;
            float v;
          if (value > 64)
          {
            MatrixOS::LED::SetColor(xy, config->color[*group].Scale((value - 63) * 3.74 + 16));
          }
          else if (value > 62 && value < 65)
          {
            Color::RgbToHsv(config->color[*group], &hue, &s, &v);
            Color color = Color::HsvToRgb(hue, 0.5, 1);
            MatrixOS::LED::SetColor(xy, color.Scale(16));
          }
          else if (value < 63)
          {
            Color::RgbToHsv(config->color[*group], &hue, &s, &v);
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
          MatrixOS::LED::SetColor(xy, config->color[*group].Scale(value * 1.87 + 16));
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    int8_t i = xy.x + xy.y * dimension.x;

    if (keyInfo->state == HOLD)
    {
      config->value2[*group][i][(config->channel[*group][i])] = config->defaultValue[*group][i];
      Device::Encoder::knobFunction(i);
    };

    return true;
  }
};
