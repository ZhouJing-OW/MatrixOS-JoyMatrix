#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"


class UIPlusMinus : public UIComponent {
 public:
  int8_t* val;
  int8_t max;
  int8_t min;
  Color color;
  bool vertical;
  bool reverse;
  std::function<void()> callback;

  UIPlusMinus(int8_t* val, int8_t max, int8_t min, Color color, bool vertical = true, std::function<void()> callback = nullptr) {
    this->val = val;
    this->max = max;
    this->min = min;
    this->color = color;
    this->vertical = vertical;
    if(this->vertical == false) this->reverse = true;
    this->callback = callback;
  }

  virtual bool Callback() {
    if (callback != nullptr)
    {
      callback();
      return true;
    }
    return false;
  }

  virtual Color GetColor() { return color; }
  virtual Dimension GetSize() { return vertical ? Dimension(1, 2) : Dimension(2, 1); }

  virtual bool Render(Point origin) {

    for (uint8_t i = 0; i < 2; i++)
    {
      
      Point xy = vertical ? origin + Point(0, i) : origin + Point(i, 0);
      
      Color color1 = color.Scale((int8_t)(((float)*val / (max - min) * 0.85 + 0.15) * 255));
      Color color2 = color.Scale((int8_t)(((1 - ((float)*val) / (max - min)) * 0.85 + 0.15) * 255));

      switch (i) {
        case 0:
          MatrixOS::LED::SetColor(xy, !reverse ? color1 : color2);
          break;
        case 1:
          MatrixOS::LED::SetColor(xy, !reverse ? color2 : color1);
          break;
      }
      
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    uint8_t i;
    if (vertical) i = xy.y;
    else i = xy.x;
    uint8_t shift = (max - min > 32) && Device::KeyPad::ShiftActived();
    shift = shift * 9;

    if (keyInfo->state == PRESSED)  
    { 
      switch (i) {
      case 0:
        if (!reverse) *val = (*val + 1 + shift) < max ? (*val + 1 + shift) : max;
        else *val = (*val - 1 - shift) > min ? (*val - 1 - shift) : min;
        // MLOGD("Value", "%d", *val);
        Callback();
        return true;
      case 1:
        if (!reverse) *val = (*val - 1 - shift) > min ? (*val - 1 - shift) : min;
        else *val = (*val + 1 + shift) < max ? (*val + 1 + shift) : max;
        MLOGD("Value", "%d", *val);
        Callback();
        return true;
      }
    } 
    return false;
  }
};