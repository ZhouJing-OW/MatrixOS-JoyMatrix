  #pragma once
  #include "MatrixOS.h"
  
  namespace MatrixOS::UIInterface
  {
    void TextScroll(string ascii, Color color, uint16_t speed = 10, bool loop = false);
    int32_t NumberSelector16x4(int16_t value, Color color, string name, int16_t lower_limit, int16_t upper_limit);
    bool ColorPicker(Color& color);
  }