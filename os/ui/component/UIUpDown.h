#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"


class UIUpDown : public UIComponent {
 public:
  int8_t* val;
  int8_t max;
  int8_t min;
  Color color;
  bool vertical;
  bool reverse;

  UIUpDown(int8_t* val, int8_t max, int8_t min, Color color, bool vertical = true, bool reverse = false) {
    this->val = val;
    this->max = max;
    this->min = min;
    this->color = color;
    this->vertical = vertical;
    this->reverse = false;
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

    if (keyInfo->state == PRESSED)  
    { 
      switch (i) {
      case 0:
        if (!reverse) *val = *val + (*val < max);
        else *val = *val - (*val > min);
        MLOGD("Octave", "%d", *val);
        return true;
      case 1:
        if (!reverse) *val = *val - (*val > min);
        else *val = *val + (*val < max);
        MLOGD("Octave", "%d", *val);
        return true;
      }
    } 
    return false;
  }
};