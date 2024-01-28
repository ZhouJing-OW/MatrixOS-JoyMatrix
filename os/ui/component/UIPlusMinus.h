#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

template<class T, class T2>
class UIPlusMinus : public UIComponent {
 public:
  T* val;
  T2 max;
  T2 min;
  Color color;
  bool vertical;
  bool loop = false;
  std::function<void()> callback;

  UIPlusMinus(T* val, T2 max, T2 min, Color color, bool vertical = true, bool loop = false, std::function<void()> callback = nullptr) {
    this->val = val;
    this->max = max;
    this->min = min;
    this->color = color;
    this->vertical = vertical;
    this->loop = loop;
    this->callback = callback;
  }

  virtual bool Callback() 
  {
    if (callback != nullptr)
    {
      callback();
      return true;
    }
    return false;
  }

  virtual Color GetColor() { return color; }
  virtual Dimension GetSize() { return vertical ? Dimension(1, 2) : Dimension(2, 1); }

  virtual bool Render(Point origin) 
  {
    for (uint8_t i = 0; i < 2; i++)
    {
      Point xy = vertical ? origin + Point(0, i) : origin + Point(i, 0);
      Color color1 = color.Scale((int8_t)(((float)*val / (max - min) * 0.85 + 0.15) * 255));
      Color color2 = color.Scale((int8_t)(((1 - ((float)*val) / (max - min)) * 0.85 + 0.15) * 255));
      if (i == 0) MatrixOS::LED::SetColor(xy, vertical ? color1 : color2);
      else MatrixOS::LED::SetColor(xy, vertical ? color2 : color1);
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) 
  {
    uint8_t shift = ((max - min > 32) && Device::KeyPad::Shift()) * 9;
    if (keyInfo->state == PRESSED)
    {    
      uint8_t i = vertical ? xy.y : xy.x;
      if (i != vertical) *val = (*val + 1 + shift) < max ? (*val + 1 + shift) : (loop ? min : max);
      else *val = (*val - 1 - shift) > min ? (*val - 1 - shift) : (loop ? max : min);
      Callback();
      return true;
    }
    return false;
  }
};