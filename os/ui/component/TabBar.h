#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class TabBar : public UIComponent {
 public:
  TabConfig* config;
  uint8_t count;
  int8_t* activeTab;
  int8_t* toggle;

  TabBar(){}
  TabBar(TabConfig* config, uint8_t count, int8_t& activeTab, int8_t& toggle) {
    this->config = config;
    this->count = count;
    this->activeTab = &activeTab;
    this->toggle = &toggle;
  }

  void Setup(TabConfig* config, uint8_t count, int8_t& activeTab, int8_t& toggle) {
    this->config = config;
    this->count = count;
    this->activeTab = &activeTab;
    this->toggle = &toggle;
  }

  virtual Color GetColor() { return config->color; }
  virtual Dimension GetSize() { return Dimension(count , 1); }

  virtual bool Render(Point origin) {

    for (uint8_t x = 0; x < count; x++)
    {        
      Point xy = origin + Point(x, 0);
      TabConfig* con = config + x;

      if (x == *activeTab)  MatrixOS::LED::SetColor(xy, con->color);
      else MatrixOS::LED::SetColor(xy, con->color.ToLowBrightness());
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    TabConfig* con = config + xy.x;
    if (keyInfo->state == RELEASED){
      if (xy.x == *activeTab) {
        MatrixOS::Component::Tab_ToggleSub(con);
        MLOGD("Tab", "tab: %d, sub: %d", *activeTab, con->subTab);
        *toggle = xy.x;
        return true;
      }
      else
      {
        *activeTab = xy.x;
        MLOGD("Tab", "tab: %d, sub: %d", *activeTab, con->subTab);
        *toggle = xy.x;
        return true; 
      }
    }

    if (keyInfo->state == HOLD){
      MatrixOS::UIInterface::TextScroll(con->name, con->color);
      return true;
    }

    return false;
  }
};
