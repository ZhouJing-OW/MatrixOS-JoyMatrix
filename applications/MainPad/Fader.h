#pragma once

#include "MatrixOS.h"


class Fader : public UIComponent {
 public:
  Dimension dimension;
  Color color;
  int32_t* value;
  int32_t value_min;
  int32_t value_max;
  bool* active;
  std::function<void()> callback;

  uint8_t piece;

  Fader(Dimension dimension, Color color, int32_t* value ,int32_t value_min, int32_t value_max, bool* active, std::function<void()> callback = nullptr) {
    this->dimension = dimension;
    this->color = color;
    this->value = value;
    this->value_min = value_min;
    this->value_max = value_max;
    this->active = active;
    this->callback = callback;

    piece = ((value_max - value_min) / (dimension.x * dimension.y));
  }

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
        Point xy = origin + Point(x, y);

        if (*value >= piece * ((x + 1) * dimension.y - y)){
          if (*active) 
            MatrixOS::LED::SetColor(xy, color);
          else 
            MatrixOS::LED::SetColor(xy, Color(0x333333));
        }
        else if (*value + piece >= piece * ((x + 1) * dimension.y - y))
        {
          MatrixOS::LED::SetColor(xy, color.Scale(256 * (*value % piece) / piece ));
        }
        else
        {
          MatrixOS::LED::SetColor(xy, Color(0x000000));
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    if (keyInfo->state == PRESSED)  
    { 
      if(*active) 
        *value = piece * ((xy.x + 1)* dimension.y - xy.y - 0.5);
      else 
        *active = !*active;

      if (Callback()){
        MLOGD("UI Fader", "Key Event Callback");
        return true;
      }
    }
    return true;
  }
};