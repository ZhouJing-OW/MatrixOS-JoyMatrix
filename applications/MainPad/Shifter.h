#pragma once

#include "MatrixOS.h"

class Shifter : public UIComponent {
 public:
  Dimension dimension;
  string name;
  Color* color1;
  Color* color2;
  uint8_t* shift;
  uint8_t max;

  Shifter(Dimension dimension, string name , Color* color1, Color* color2, uint8_t* shift , uint8_t max = 0) {
    this->dimension = dimension;
    this->name = name;
    this->color1 = color1;
    this->color2 = color2;
    this->shift = shift;
    this->max = max;
  }

  virtual string GetName() { return name; }
  virtual Color GetColor() { return *color1; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Render(Point origin) {
    for (uint16_t y = 0; y < dimension.y; y++)
    {
      for (uint16_t x = 0; x < dimension.x; x++){
        Point xy = origin + Point(x, y);
        uint8_t n = y * dimension.x + x;
        if (max && n >= max)
          MatrixOS::LED::SetColor(xy, Color(0x000000));
        else
          MatrixOS::LED::SetColor(xy, (n == *shift) ? *color2 : *color1);
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    if (keyInfo->state == HOLD)
    {
      MatrixOS::UIInterface::TextScroll(name, GetColor());
      return true;
    }
    int8_t n = xy.y * dimension.x + xy.x;
    if (keyInfo->state == PRESSED)
    { if (!(max && n >= max)) *shift = n; }
    return true;
  }
};