#pragma once
#include "MatrixOS.h"
#include <cmath>
#include "../data/4pxNumber.h"
#include "UIComponent.h"
#include <vector>


class UI4pxKnobVar : public UIComponent {
 public:
  std::vector<KnobConfig*> knobPtr;
  std::vector<int16_t> knobPrv;
  uint8_t digits;
  uint8_t spacing;
  uint8_t activeKnob = 0;
  string btnName[4] = {"+10", "+1", "-1", "-10"};

  UI4pxKnobVar(vector<KnobConfig*> knobPtr, uint8_t digits, uint8_t spacing = 1) {
    this->knobPtr = knobPtr;
    this->digits = digits;
    this->spacing = spacing;
    for (uint8_t i = 0; i <knobPtr.size() ; i++)
    {
      if(knobPtr[i] != nullptr)
        knobPrv.push_back(knobPtr[i]->byte2);
      else
        knobPrv.push_back(0);
    }

  }

  virtual Dimension GetSize() { return Dimension(digits * 3 + (digits - 1) * (digits > 0) * spacing + 2, 4); }
  virtual Color GetColor() { return (knobPtr.size() > 0) ? knobPtr[activeKnob]->color : COLOR_BLANK; };

  void Render4pxNumber(Point origin, Color color, uint8_t value) {
    // MLOGD("4PX", "Num: %d, render at %d-%d", value, origin.x, origin.y);
    if (value < 11 /*&& value >= 0*/)
    {
      for (int8_t x = 0; x < 3; x++)
      {
        for (int8_t y = 0; y < 4; y++)
        { MatrixOS::LED::SetColor(origin + Point(x, 3 - y), bitRead(number4px[value][x], y) ? color : COLOR_BLANK); }
      }
    }
  }

  virtual bool Render(Point origin) {
    ActiveKnob();
    if (knobPtr[activeKnob] == nullptr)
      return false;
    
    int16_t value = knobPtr[activeKnob]->byte2;
    uint8_t sig_figure = int(log10(value) + 1);
    Point render_origin = origin;
    // MLOGD("4PX", "Render %d, sigfig %d", *value, sig_figure);
    for (int8_t digit = digits - 1; digit >= 0; digit--)
    {
      if (digit < sig_figure || digit == 0)
      { Render4pxNumber(render_origin, GetColor(), (int)(value/ std::pow(10, digit)) % 10); }
      else
        Render4pxNumber(render_origin, COLOR_BLANK, 10);
      render_origin = render_origin + Point(3 + spacing, 0);
    }
    RenderplusMinus(origin + Point(GetSize().x - 1, 0));
    return true;
  }

  bool RenderplusMinus(Point origin) {
    Color thisColor = COLOR_WHITE;
    MatrixOS::LED::SetColor(origin + Point(0, 0), thisColor);
    MatrixOS::LED::SetColor(origin + Point(0, 1), thisColor);
    MatrixOS::LED::SetColor(origin + Point(0, 2), thisColor.ToLowBrightness());
    MatrixOS::LED::SetColor(origin + Point(0, 3), thisColor.ToLowBrightness());
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    if(xy.x != GetSize().x - 1)
      return false;

    if (keyInfo->state == PRESSED)
    {
      switch (xy.y)
      {
      case 0:
        knobPtr[activeKnob]->byte2 = knobPtr[activeKnob]->byte2 + 10 < knobPtr[activeKnob]->max ? knobPtr[activeKnob]->byte2 + 10 : knobPtr[activeKnob]->max;
        break;
      case 1:
        knobPtr[activeKnob]->byte2 += (knobPtr[activeKnob]->byte2 < knobPtr[activeKnob]->max);
        break;
      case 2:
        knobPtr[activeKnob]->byte2 -= (knobPtr[activeKnob]->byte2 > knobPtr[activeKnob]->min);
        break;
      case 3:
        knobPtr[activeKnob]->byte2 = knobPtr[activeKnob]->byte2 - 10 > knobPtr[activeKnob]->min ? knobPtr[activeKnob]->byte2 - 10 : knobPtr[activeKnob]->min;
        break;
      }
      return true;
    }

    if (keyInfo->state == HOLD)
    {
      MatrixOS::UIInterface::TextScroll(btnName[xy.y], GetColor());
      return true;
    }

    return false;
  }

  void ActiveKnob(){
    for(uint8_t i = 0; i < knobPtr.size(); i++)
    {
      if(knobPtr[i] != nullptr)
      {
        if(knobPtr[i]->byte2 != knobPrv[i])
        {
          activeKnob = i;
          knobPrv[i] = knobPtr[i]->byte2;
          return;
        }
      }
    }
  }
};

