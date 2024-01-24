#pragma once
#include "UI/UI.h"
#include <Functional>

// TODO add negative support?
template <typename T>
class UI4pxNumberWithColorFunc : public UI4pxNumber<T>{
 public:
  std::function<Color()> color_func;
  UI4pxNumberWithColorFunc(std::function<Color()> color_func, uint8_t digits, T* value, bool add1 = false, uint8_t spacing = 1)
      : UI4pxNumber<T>(color_func(), digits, value, add1, spacing) {
    this->color_func = color_func;
  }

  virtual Color GetColor() { return color_func(); };
  virtual Color GetAlternativeColor() { return color_func(); };
};
