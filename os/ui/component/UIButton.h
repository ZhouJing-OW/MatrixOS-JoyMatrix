#pragma once
#include "../UIInterfaces.h"

class UIButton : public UIComponent {
 public:
  string name;
  Color color;
  std::function<void()> callback;
  std::function<void()> hold_callback;
  std::function<void()> press_callback;
  std::function<void()> release_callback;

  UIButton(string name, Color color, std::function<void()> callback = nullptr,
           std::function<void()> hold_callback = nullptr, 
           std::function<void()> press_callback = nullptr, 
           std::function<void()> release_callback = nullptr) {
    this->name = name;
    this->color = color;
    this->callback = callback;
    this->hold_callback = hold_callback;
    this->press_callback = press_callback;
    this->release_callback = release_callback;
  }

  virtual string GetName() { return name; }
  virtual Color GetColor() { return color; }
  virtual Dimension GetSize() { return Dimension(1, 1); }

  virtual bool Callback() {
    if (callback != nullptr)
    {
      callback();
      return true;
    }
    return false;
  }
  virtual bool HoldCallback() {
    if (hold_callback)
    {
      hold_callback();
      return true;
    }
    return false;
  }
  virtual bool PressCallback() {
    if (press_callback)
    {
      press_callback();
      return true;
    }
    return false;
  }
  virtual bool ReleaseCallback() {
    if (release_callback)
    {
      release_callback();
      return true;
    }
    return false;
  }
  virtual bool Render(Point origin) {
    MatrixOS::LED::SetColor(origin, GetColor());
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    if (keyInfo->state == RELEASED && keyInfo->hold == false)
    {
      if (Callback())
      {
        MLOGD("UI Button", "Key Event Callback");
        // MatrixOS::KEYPAD::Clear();
        return true;
      }
    }
    else if (keyInfo->state == HOLD)
    {
      if (HoldCallback())
      {
        // MatrixOS::KEYPAD::Clear();
        return true;
      }
      else
      {
        MatrixOS::UIInterface::TextScroll(GetName(), GetColor());
        return true;
      }
    }
    else if (keyInfo->state == PRESSED)
    {
      if (PressCallback())
      {
        // MatrixOS::KEYPAD::Clear();
        return true;
      }
    }
    else if (keyInfo->state == RELEASED)
    {
      if (ReleaseCallback())
      {
        // MatrixOS::KEYPAD::Clear();
        return true;
      }
    }
    return false;
  }
};
