#pragma once
#include "UIButton.h"

class UIButtonDimmableLarge : public UIButton {
 public:
  std::function<bool()> dim_func;  // If this returns false, then dim button
  Dimension dimension;

  UIButtonDimmableLarge(string name, Color color, Dimension dimension, std::function<bool()> dim_func, std::function<void()> callback = nullptr,
                   std::function<void()> hold_callback = nullptr)
      : UIButton(name, color, callback, hold_callback) {
    this->dim_func = dim_func;
    this->dimension = dimension;
  }
  virtual Dimension GetSize() { return dimension; }
  virtual Color GetColor() { return color.ToLowBrightness(dim_func()); }
  
  virtual bool Render(Point origin) {
    for (uint16_t x = 0; x < dimension.x; x++)
    {
      for (uint16_t y = 0; y < dimension.y; y++)
      { MatrixOS::LED::SetColor(origin + Point(x, y), GetColor()); }
    }
    return true;
  }

};
