#pragma once

#include "MatrixOS.h"

class MainOctaveShifter : public UIComponent {
 public:
  string name;
  uint8_t count;
  MainPadConfig* configs;
  uint8_t* activeConfig;

  MainOctaveShifter(string name, uint8_t count, MainPadConfig* configs, uint8_t* activeConfig) {
    this->name = name;
    this->count = count;
    this->configs = configs;
    this->activeConfig = activeConfig;
  }

  virtual string GetName() { return name; }
  virtual Color GetColor() { return configs[*activeConfig].color; }
  virtual Dimension GetSize() { return Dimension(count, 1); }

  virtual bool Render(Point origin) {
    for (uint16_t octave = 0; octave < count; octave++)
    {
      // Maybe allow different direction
      Point xy = origin + Point(octave, 0);
      MatrixOS::LED::SetColor(xy, (octave == configs[*activeConfig].octave) ? configs[*activeConfig].rootColor : configs[*activeConfig].color);
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    if (keyInfo->state == HOLD)
    {
      MatrixOS::UIInterface::TextScroll(name, GetColor());
      return true;
    }
    int8_t octave = xy.x;
    if (keyInfo->state == PRESSED)
    { configs[*activeConfig].octave = octave; }
    return true;

    
  }
};