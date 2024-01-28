#pragma once

#include "UIComponent.h"
//#include "MatrixOS.h"

class ScrollBar : public UIComponent {
public:
  Dimension dimension;
  const Color* color;
  int8_t* num;
  uint8_t count;
  uint8_t sliderLength;
  Point position = Point(0, 0);

  ScrollBar()
  {
    this->dimension = Dimension(0, 0);
    this->color = nullptr;
    this->num = nullptr;
  }

  ScrollBar(int8_t* num, uint8_t count, const Color* color, uint8_t sliderLength = 1, Point position = Point(0, 0)) {
    Setup(num, count, color, sliderLength, position);
  }

  void Setup(int8_t* num, uint8_t count, const Color* color, uint8_t sliderLength = 1, Point position = Point(0, 0)) {
    this->dimension = Dimension( (count / 4) + 1, 4 );
    this->num = num;
    this->count = count;
    this->color = color;
    this->sliderLength = sliderLength;
    this->position = position;  
  }

  virtual Color GetColor() { return *color; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Render(Point origin)
  {
    for (uint8_t x = 0; x < dimension.x; x++)
    {
      for(uint8_t y = 0; y < dimension.y; y++)
      {
        Point xy = Point(x, y) + origin;
        uint8_t i = dimension.y * x + y;
        if (i < count)
        {
          if (i == *num || i == *num + sliderLength - 1)
            MatrixOS::LED::SetColor(xy, COLOR_WHITE);
          else
          {
            Color thisColor = *(color + i);
            MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
          }
        }
        else
          MatrixOS::LED::SetColor(xy, COLOR_BLANK); 
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
  {
    uint8_t i = dimension.y * xy.x + xy.y;
    if (keyInfo->state == PRESSED)
    {
      if(i < count - (sliderLength - 1))
        *num = i;
      else if(i >= count - (sliderLength - 1) && i < count)
        *num = count - sliderLength;
      return true;
    }
    else 
      return false;
  }
};