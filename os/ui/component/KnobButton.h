#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class KnobButton : public UIComponent {
 public:
  Dimension dimension;
  std::vector<KnobConfig*> knobPtr; 
  std::function<void()> callback;
  bool activeMode;
  int8_t activePoint = -ENCODER_NUM;
  uint8_t count;

  Point position = Point(0, 0);
  const uint8_t lowBrightness = 16;

  KnobButton(){};
  KnobButton(Dimension dimension, uint8_t count, bool activeMode = false, std::function<void()> callback = nullptr) {
    Setup(dimension, count, activeMode, callback);
  };

  void Setup(Dimension dimension, uint8_t count, bool activeMode = false, std::function<void()> callback = nullptr){
    this->dimension = dimension;
    this->count = count;
    this->callback = callback;
    this->activeMode = activeMode;

    knobPtr.reserve(count);
    for (uint8_t n = 0; n < count; n++)
    {
      knobPtr.push_back(nullptr);
    }
  }

  void SetKnobs(std::vector<KnobConfig*>& knobs)
  {
    knobPtr.reserve(knobs.size());
    for (uint8_t n = 0; n < knobs.size() && n < count; n++)
      knobPtr.push_back(knobs[n]);
  }

  void SetKnobs(std::vector<uint16_t>& pos, int8_t activePoint = -ENCODER_NUM)
  {
    MatrixOS::KnobCenter::GetKnobPtrs(pos, knobPtr);
    this->activePoint = activePoint;
    if(activePoint >= 0)
      activeMode = true;
    else
      activeMode = false;
    SetKnobBar();
  }

  void SetActivePoint(int8_t activePoint = 0)
  {
    this->activePoint = activePoint;
    if(activePoint >= 0)
      activeMode = true;
    else
      activeMode = false;
    SetKnobBar();
  }

  virtual bool Callback() {
    if (callback != nullptr) {
      callback();
      return true;
    }
    return false;
  }

  virtual Color GetColor() {  if(knobPtr[0] != nullptr) return knobPtr[0]->color; else return COLOR_WHITE; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Render(Point origin) {
    position = origin;

    for (uint8_t x = 0; x < dimension.x; x++)
    {
      for (uint8_t y = 0; y < dimension.y; y++)
      {
        Point xy = origin + Point(x, y);
        uint8_t i = y * dimension.x + x;
        bool active = (i >= activePoint) && (i < activePoint + ENCODER_NUM) && activeMode;
        
        if (knobPtr[i] != nullptr && i < count)
        {
          int16_t val = knobPtr[i]->byte2;
          int16_t range = knobPtr[i]->max - knobPtr[i]->min + 1;
          int16_t halfRange = range / 2;
          Color color = knobPtr[i]->color;

          if (knobPtr[i]->def == halfRange || knobPtr[i]->def == halfRange + range % 2 - 1)  // middleMode
          {
            if (val > halfRange + 1)  // right
            {
              Color thisColor = color.Blink_Color(active, COLOR_WHITE);
              thisColor = thisColor.Scale(val, halfRange, knobPtr[i]->max, lowBrightness);
              MatrixOS::LED::SetColor(xy, knobPtr[i]->lock ? thisColor : thisColor.Blink_Key(Device::KeyPad::fnState));
            }
            else if ((val >= halfRange - 1) && (val <= halfRange + 1))  // middle
            {
              Color thisColor = color.Rotate(90);
              thisColor = thisColor.Blink_Color(active, COLOR_WHITE);
              thisColor = thisColor.Scale(lowBrightness);
              MatrixOS::LED::SetColor(xy, knobPtr[i]->lock ? thisColor : thisColor.Blink_Key(Device::KeyPad::fnState));
            }
            else if (val < halfRange - 1)  // left
            {
              Color thisColor = color.Invert();
              thisColor = thisColor.Blink_Color(active, COLOR_WHITE);
              thisColor = thisColor.Scale(halfRange - val, knobPtr[i]->min, halfRange, lowBrightness);
              MatrixOS::LED::SetColor(xy, knobPtr[i]->lock ? thisColor : thisColor.Blink_Key(Device::KeyPad::fnState));
            }
          }
          else  // Regular mode
          {
            Color thisColor = color.Blink_Color(active, COLOR_WHITE);
            thisColor = thisColor.Scale(val, knobPtr[i]->min, knobPtr[i]->max, lowBrightness);
            MatrixOS::LED::SetColor(xy, knobPtr[i]->lock ? thisColor : thisColor.Blink_Key(Device::KeyPad::fnState));
          }
        }
        else
          MatrixOS::LED::SetColor(xy, COLOR_BLANK);
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    uint8_t i = xy.y * dimension.x + xy.x;
    if(knobPtr[i] != nullptr && i < count)
    {
      bool active = (i >= activePoint) && (i < activePoint + ENCODER_NUM);

      if (keyInfo->state == PRESSED)
      {
        if((Device::KeyPad::fnState.active()) && knobPtr[i]->lock == false) 
        {
          MatrixOS::Component::Knob_Setting(knobPtr[i], false);
          return true;
        }
        Device::AnalogInput::UseDial(xy + position, knobPtr[i]);
        if (activeMode && !active) 
        {
          activePoint = i - i % ENCODER_NUM;
          std::vector<KnobConfig*> tempKnob;
          tempKnob.reserve(ENCODER_NUM);
          for (uint8_t n = 0; n < ENCODER_NUM; n++)
          {
            if (activePoint + n < count)
              tempKnob.push_back(knobPtr[activePoint + n]);
            else
              tempKnob.push_back(nullptr);
          }
          MatrixOS::KnobCenter::SetKnobBar(tempKnob);
          Callback();
        }
      }
      return true;
    }
    return false;
  }

  private:
  void SetKnobBar()
  {
    if (activePoint >= 0 && activeMode == true)
    {
      std::vector<KnobConfig*> tempKnob;
      tempKnob.reserve(count);
      for (uint8_t n = 0; n < count; n++)
      {
        if (activePoint + n < count)
          tempKnob.push_back(knobPtr[activePoint + n]);
        else
          tempKnob.push_back(nullptr);
      }
      MatrixOS::KnobCenter::SetKnobBar(tempKnob);
    }
  }
};

