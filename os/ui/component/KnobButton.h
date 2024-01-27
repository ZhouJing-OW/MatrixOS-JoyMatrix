#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"
#include "KnobBar.h"

class KnobButton : public UIComponent {
 public:
  Dimension dimension;
  std::vector<KnobConfig*> knobPt;  // When used as a knob bar, this pointer points to the encoder. 
  std::vector<KnobConfig> knob;     // When used as a knob bar, The encoder pointer points to the container
  std::vector<uint16_t> knobID;
  std::vector<int8_t> knobValPrev;
  std::function<void()> callback;
  std::unordered_map<int8_t, int8_t> smoothList;
  bool enableActivePoint;
  int8_t activePoint = -ENCODER_NUM;
  uint8_t count;

  Point position = Point(0, 0);
  bool KnobBarMode;
  uint8_t displayNum;
  uint32_t BarDisplayTimer = 0;
  const uint16_t BarDisplayDuration = 700;

  KnobButton(){};
  KnobButton(Dimension dimension, uint8_t count, bool enableActivePoint = false, bool KnobBarMode = false, std::function<void()> callback = nullptr) {
    Setup(dimension, count, enableActivePoint, KnobBarMode, callback);
  };

  void Setup(Dimension dimension, uint8_t count, bool enableActivePoint = false, bool KnobBarMode = false, std::function<void()> callback = nullptr){
    this->dimension = dimension;
    this->count = count;
    this->callback = callback;
    this->enableActivePoint = enableActivePoint;
    this->KnobBarMode = KnobBarMode;
    for (uint8_t n = 0; n < count; n++)
    {
      KnobConfig tempKnob;
      knob.push_back(tempKnob);
      knobID.push_back(0xFFFF);
      knobValPrev.push_back(-1);
    }
    for(uint8_t n = 0; n < count; n++){
      knobPt.push_back(&knob[n]);
    }
  }

  void SetPtr(KnobConfig* knob, uint8_t n){
    knobPt[n] = knob;
    knobValPrev[n] = knob->byte2;
  }

  void SetToDefault()
  {

  }

  bool Disable(uint8_t n) {
    if (n < count) {
      this->knob[n].enable = false;
      this->knobID[n] = 0xFFFF; // do not save this knob 
      return true;
    }
    return false;
  }

  void DisableALL(){
    for (uint8_t n = 0; n < count; n++){
      this->Disable(n);
    }
  }

  virtual bool Callback() {
    if (callback != nullptr) {
      callback();
      return true;
    }
    return false;
  }

  virtual Color GetColor() { return knobPt[0]->color; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Render(Point origin) {
    position = origin;
    Smooth(200);

    if(KnobBarMode){
      for (uint8_t i = 0; i < count; i++){
        if (knobValPrev[i] != knobPt[i]->byte2){
          BarDisplayTimer = MatrixOS::SYS::Millis() + BarDisplayDuration;
          knobValPrev[i] = knobPt[i]->byte2;
          displayNum = i;
          break;
        }
      }
    }

    KnobConfig* dialKnob = Device::AnalogInput::GetDialKnob();
    if (dialKnob != nullptr) return BarDisplay(origin,  dialKnob);
    else if (BarDisplayTimer > MatrixOS::SYS::Millis()) return BarDisplay(origin,  knobPt[displayNum]);
    else BarDisplayTimer = 0;

    for (uint8_t x = 0; x < dimension.x; x++) 
    { 
      for(uint8_t y = 0; y < dimension.y; y++) 
      { 
        Point xy = origin + Point(x, 0);
        uint8_t i = y * dimension.x + x;


        if(knobPt[i] != nullptr && knobPt[i]->enable)
        {
          int8_t val = knobPt[i]->byte2;
          bool active = enableActivePoint && ((i >= activePoint) && (i < activePoint + ENCODER_NUM));
          Color thisColor = knobPt[i]->color.Blink(active, COLOR_WHITE);
          
          uint8_t LowLight = 16;
          float ratio = (256 - LowLight) / 2;
          int8_t range = knobPt[i]->max - knobPt[i]->min;
          int8_t halfRange = range / 2;

          if (i < count)
          { 
            if (knobPt[i]->def > halfRange - 1 && knobPt[i]->def < halfRange + 1) // The default value is centered
            {
                float hue; float s; float v;
              if (val >= halfRange + 2)
              {
                uint8_t halfVal = val - halfRange < 0 ? 0 : val - halfRange;
                uint8_t scale = (halfVal * ratio / halfRange + LowLight) > 255 ? 255 : (halfVal * ratio / halfRange + LowLight);
                MatrixOS::LED::SetColor(xy, knobPt[i]->lock ? thisColor.Scale(scale) : thisColor.Scale(scale).Blink(Device::KeyPad::fnState));
              }
              else if (val >= halfRange - 2 && val <= halfRange + 2)
              {
                Color::RgbToHsv(thisColor, &hue, &s, &v);
                Color tempColor = Color::HsvToRgb(hue, 0.5, 1);
                MatrixOS::LED::SetColor(xy, knobPt[i]->lock ? tempColor.Scale(LowLight) : tempColor.Scale(LowLight).Blink(Device::KeyPad::fnState));
              }
              else if (val < halfRange - 2)
              {
                Color::RgbToHsv(thisColor, &hue, &s, &v);
                if(0.5 - hue > 0) hue = 0.5 - hue;
                else hue = 1.5 - hue;
                Color tempColor = Color::HsvToRgb(hue, s, v);
                
                uint8_t scale = (((halfRange - val) * ratio / halfRange + LowLight) > 255 ? 255 : ((halfRange - val) * ratio / halfRange + LowLight));
                MatrixOS::LED::SetColor(xy, knobPt[i]->lock ? tempColor.Scale(scale) : tempColor.Scale(scale).Blink(Device::KeyPad::fnState));
              }
            }
            else
            {
              uint8_t scale = (val * ratio / range + LowLight) > 255 ? 255 : (val * ratio / range + LowLight);
              MatrixOS::LED::SetColor(xy, knobPt[i]->lock ? thisColor.Scale(scale) : thisColor.Scale(scale).Blink(Device::KeyPad::fnState));
            }
          } else {
            MatrixOS::LED::SetColor(xy, COLOR_BLANK);
          }
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    uint8_t i = xy.y * dimension.x + xy.x;
    
    if(i < knobPt.size() && knobPt[i] != nullptr)
    {
      bool active = (i >= activePoint) && (i < activePoint + ENCODER_NUM);
      if ((enableActivePoint && active) || !enableActivePoint)
      { 
        if(Device::KeyPad::ShiftActived() && keyInfo->state == PRESSED && knobPt[i]->enable == true)
        {
          smoothList.emplace(i, knobPt[i]->def);
          // knobPt[i]->byte2 = knobPt[i]->def;
          // MatrixOS::Component::Knob_Function(knobPt[i]);
          return true;
        }

        if (keyInfo->state == PRESSED && knobPt[i]->enable == true){
          Device::AnalogInput::UseDial(xy + position, knobPt[i]);
          if((Device::KeyPad::fnState == ACTIVATED || Device::KeyPad::fnState == HOLD) && knobPt[i]->lock == false) 
          {
            MatrixOS::Component::Knob_Setting(knobPt[i], false);
            return true;
          }
        }
        return true;
      }

      if (enableActivePoint && !active)
      {
        if (keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          activePoint = i - i % ENCODER_NUM;
          Callback();
        }
      }
      return true;
    }
    return false;
  }

  bool BarDisplay(Point origin , KnobConfig* displayKnob){
    for (uint8_t x = 0; x < dimension.x; x++)
    {
      Point xy = Point(x, 0) + origin;
      Color color = displayKnob->color;
      float hue; float s; float v; Color::RgbToHsv(color, &hue, &s, &v);
      if(0.5 - hue > 0) hue = 0.5 - hue; else hue = 1.5 - hue;
      Color contColor = Color::HsvToRgb(hue, s, v);

      uint8_t full = (displayKnob->max - displayKnob->min + 1);
      uint8_t half = (full / 2);
      float value = displayKnob->byte2;
      float divide = dimension.x;
      float piece = (full / divide);
      bool middleMode = (displayKnob->def >= half - 1 && displayKnob->def <= half);
      uint8_t thisPoint = x + 1;
      if (middleMode) 
      {
        if(value == half - 1 || value == half) 
        {
          if (x == dimension.x / 2 - 1) MatrixOS::LED::SetColor(xy, contColor.Scale(16));
          else if (x == dimension.x / 2) MatrixOS::LED::SetColor(xy, color.Scale(16));
          else MatrixOS::LED::SetColor(xy, COLOR_BLANK);
          continue;
        } 
        else if(value > half && x < (dimension.x / 2)) 
        {
          MatrixOS::LED::SetColor(xy, COLOR_BLANK);
          continue;
        }
        else if(value < half - 1)
        {
          if(x >= (dimension.x / 2)) 
          {
            MatrixOS::LED::SetColor(xy, COLOR_BLANK);
            continue;
          }
          thisPoint = dimension.x / 2 - x;
          value = half - value;
          color = contColor;
        }
      }

      if (thisPoint * piece >= value && (thisPoint - 1) * piece < value)
      {
        uint8_t scale = (int)(239 * (value - (thisPoint - 1) * piece) / piece + 16);
        MatrixOS::LED::SetColor(xy, color.Scale(scale));
      }
      else if (thisPoint * piece <= value)
      {
        MatrixOS::LED::SetColor(xy, color);
      } 
      else 
        MatrixOS::LED::SetColor(xy, COLOR_BLANK);
    }
    return true;
  }

  void Smooth(uint16_t ms)
  {
    uint8_t step = 127 / (ms / (1000 / Device::fps));
    for (auto it = smoothList.begin(); it != smoothList.end(); it++)
    {
      int8_t i = it->first;
      int8_t target = it->second;
      if (knobPt[i]->byte2 < target) knobPt[i]->byte2 = knobPt[i]->byte2 + step > target ? target : knobPt[i]->byte2 + step;
      if (knobPt[i]->byte2 > target) knobPt[i]->byte2 = knobPt[i]->byte2 - step < target ? target : knobPt[i]->byte2 - step;
      MatrixOS::Component::Knob_Function(knobPt[i]);
      if (knobPt[i]->byte2 == target) {
        smoothList.erase(it);
        break;
      }
    }
  }
};

