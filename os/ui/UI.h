#pragma once

#include <string>
#include <functional>
#include <list>
#include <map>
#include <stdarg.h>
#include "UIComponents.h"
#include "UIInterfaces.h"
#include "component/ScrollBar.h"

#define UI_DEFAULT_FPS 30
#define UI_POP_DELAY 400

class UI {
 public:
  string name;
  Color nameColor;
  int8_t status = 0;
  bool displayScrollBar;

  bool newLedLayer = true;
  bool disableExit = false;
  bool needRender = false;

  Timer uiTimer;
  uint32_t uiUpdateMS = 1000 / UI_DEFAULT_FPS;

  std::function<void()>* setup_func = nullptr;
  std::function<void()>* loop_func = nullptr;
  std::function<void()>* render_func = nullptr;
  std::function<void()>* end_func = nullptr;
  std::function<bool(KeyEvent*)>* key_event_handler = nullptr;
  std::map<Point, UIComponent*> uiComponentMap;
  ScrollBar scrollBar;

  UI(){};
  UI(string name, Color color = Color(0xFFFFFF), bool newLedLayer = true);

  void Start();

  virtual void Setup() {
    if (setup_func)
      (*setup_func)();
  };
  virtual void Loop() {
    if (loop_func)
      (*loop_func)();
  };
  virtual void Render() {
    if (render_func)
      (*render_func)();
  };
  virtual void End() {
    if (end_func)
      (*end_func)();
  };

  void Exit();
  void LoopTask();

  void GetKey();
  virtual bool CustomKeyEvent(KeyEvent* keyEvent) { 
    if (key_event_handler)
      return (*key_event_handler)(keyEvent);
    return false; 
  };  // Return true to skip UIKeyEvent

  void SetSetupFunc(std::function<void()> setup_func);
  void SetLoopFunc(std::function<void()> loop_func);
  void SetEndFunc(std::function<void()> end_func);
  void SetKeyEventHandler(std::function<bool(KeyEvent*)> key_event_handler);
  void SetScrollBar(int8_t* num, uint8_t count, const Color* color, uint8_t sliderLength = 1, Point position = Point(0, 0));

  void AddUIComponent(UIComponent* uiComponent, Point xy);
  void AddUIComponent(UIComponent* uiComponent, uint16_t count, ...);
  void RemoveUIComponent(Point xy);

  void AllowExit(bool allow);
  void ClearUIComponents();
  void SetFPS(uint16_t fps);

 private:
  void RenderUI();
  void UIEnd();
  void UIKeyEvent(KeyEvent* keyEvent);
  // void PostCallbackCleanUp();
};