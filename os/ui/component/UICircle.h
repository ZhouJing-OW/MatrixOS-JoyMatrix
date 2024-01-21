#pragma once
#include "../UIInterfaces.h"

class UICircle : public UIComponent {
 public:
  string name;
  std::function<void()> callback;
  const uint8_t circle[4] = {0b0110, 0b1001, 0b1001, 0b0110};

  UICircle(string name, std::function<void()> callback = nullptr) {
    this->name = name;
    this->callback = callback;
  }

  virtual string GetName() { return name; }
  virtual Color GetColor() { return COLOR_LIME; }
  virtual Dimension GetSize() { return Dimension(4, 4); }

  virtual bool Callback() {
    if (callback != nullptr)
    {
      callback();
      return true;
    }
    return false;
  };

  virtual bool Render(Point origin) {
    
    for(uint8_t x = 0; x < 4; x++) {
      for(uint8_t y = 0; y < 4; y++) {
        Point xy = origin + Point(x, y);
        if (circle[y] >> x & 1) MatrixOS::LED::SetColor(xy, GetColor());
        else MatrixOS::LED::SetColor(xy, COLOR_BLANK);
      }
    } 
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    if (keyInfo->state == RELEASED) {
      if (Callback()) {
        MLOGD("UI Button", "Key Event Callback");
        // MatrixOS::KEYPAD::Clear();
        return true;
      } 
      return false;
    } else if (keyInfo->state == HOLD) {
        MatrixOS::UIInterface::TextScroll(GetName(), GetColor());
        return true;
    }
    return false;
  }
};
