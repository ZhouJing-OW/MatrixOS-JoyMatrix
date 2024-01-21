#pragma once
#include "../UIInterfaces.h"

class UIColorSelector : public UIComponent {
 public:
  Dimension dimension;
  Color* color;
  std::function<void()> callback;
  const Color colors[12] = {
    COLOR_RED, COLOR_PINK, COLOR_ORANGE,
    COLOR_YELLOW, COLOR_GREEN, COLOR_LIME,
    COLOR_GUPPIE, COLOR_CYAN, COLOR_AZURE, 
    COLOR_BLUE, COLOR_VIOLET, COLOR_FUCHSIA
  };

  UIColorSelector(Dimension dimension, Color* color, std::function<void()> callback = nullptr) {
    this->dimension = dimension;
    this->color = color;
    this->callback = callback;
  }

  virtual Color GetColor() { return *color; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Callback() {
    if (callback != nullptr)
    {
      callback();
      return true;
    }
    return false;
  };

  virtual bool Render(Point origin) {
    
    for(uint8_t x = 0; x < dimension.x; x++) {
      for(uint8_t y = 0; y < dimension.y; y++) {
        Point xy = origin + Point(x, y);
        uint8_t i = y * dimension.x + x;
        if (i < 12) {
          if (*color == colors[i]) {
            MatrixOS::LED::SetColor(xy, colors[i]);
          } else {
            Color tempColor = colors[i];
            MatrixOS::LED::SetColor(xy, tempColor.ToLowBrightness());
          }
        }
        else MatrixOS::LED::SetColor(xy, COLOR_BLANK);
      }
    } 
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    if (keyInfo->state == PRESSED) {
      uint8_t i = xy.y * dimension.x + xy.x;
      *color = colors[i];
      if (Callback())
      {
        MLOGD("UI Button", "Key Event Callback");
        // MatrixOS::KEYPAD::Clear();
        return true;
      }
      return false;
    } else if (keyInfo->state == HOLD) {
        MatrixOS::UIInterface::TextScroll("Color Selector", GetColor());
        return true;
    }
    return false;
  }
};
