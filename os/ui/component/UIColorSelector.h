#pragma once
#include "../UIInterfaces.h"

class UIColorSelector : public UIComponent {
 public:
  Color* color;
  std::function<void()> callback;
  const Color colors[4][4] = {{Color(RED),     Color(ORANGE),   Color(GOLD),     Color(YELLOW)},
                              {Color(MAGENTA), Color(WHITE),    Color(WHITE),    Color(LAWN)},
                              {Color(VIOLET),  Color(WHITE),    Color(WHITE),    Color(GREEN)},
                              {Color(PURPLE),  Color(BLUE),     Color(CYAN),     Color(TURQUOISE)},};

  UIColorSelector(Color* color, std::function<void()> callback = nullptr) {
    this->color = color;
    this->callback = callback;
  }

  virtual Color GetColor() { return *color; }
  virtual Dimension GetSize() { return Dimension(4,4); }

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

        if (xy == Point(1, 1) || xy == Point(1, 2) || xy == Point(2, 1) || xy == Point(2, 2) ){
          MatrixOS::LED::SetColor(xy, *color);
        } else {
          MatrixOS::LED::SetColor(xy, colors[y][x]);
        } 
      }
    } 
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    
    if (keyInfo->state == PRESSED) {
      if (!(xy == Point(1, 1) || xy == Point(1, 2) || xy == Point(2, 1) || xy == Point(2, 2)) ){
        *color = colors[xy.y][xy.x];
      }
      
      if (Callback())
      {
        MLOGD("UI Button", "Key Event Callback");
        // MatrixOS::KEYPAD::Clear();
        return true;
      }
      return false;
    } 
    
    if (keyInfo->state == HOLD) {
        MatrixOS::UIInterface::TextScroll("Color Picker", colors[xy.y][xy.x]);
        return true;
    }
    return false;
  }
};
