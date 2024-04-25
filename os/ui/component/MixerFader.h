#pragma once
#include "MatrixOS.h"
#include "KnobButton.h"

class MixerFader : public UIComponent {
 public:
  Dimension dimension;
  std::vector<KnobConfig*> knobPtr;  // When used as a knob bar, this pointer points to the encoder. 
  std::function<void()> callback;
  std::unordered_map<int8_t, int16_t> smoothList;
  bool activeMode;
  bool vertical;
  int8_t activePoint = -ENCODER_NUM;
  uint8_t count;
  Point position = Point(0, 0);

  MixerFader(){};
  MixerFader(Dimension dimension, uint8_t count, bool activeMode = true,bool vertical = true, std::function<void()> callback = nullptr){
    Setup(dimension, count, activeMode, vertical, callback);
  }

  void Setup(Dimension dimension, uint8_t count, bool activeMode = false, bool vertical = true, std::function<void()> callback = nullptr){
    this->dimension = dimension;
    this->count = count;
    this->callback = callback;
    this->activeMode = activeMode;
    this->vertical = vertical;

    knobPtr.reserve(count);
    for (uint8_t n = 0; n < count; n++)
    {
      knobPtr.push_back(nullptr);
    }
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

  virtual Color GetColor() { return knobPtr[0]->color; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Render(Point origin) {
    position = origin;
    // Smooth(200);

    for (uint8_t y = 0; y < dimension.y; y++){ 
      for (uint8_t x = 0; x < dimension.x; x++){
        Point xy = origin + Point(x, y);
        uint8_t i = vertical ? x : y;
        bool active = activeMode && (i >= activePoint) && (i < activePoint + ENCODER_NUM);
        if(knobPtr[i] != nullptr && i < count)
        {
          float value = knobPtr[i]->Value();
          float divide = vertical ? dimension.y : dimension.x;
          float piece = ((knobPtr[i]->max - knobPtr[i]->min + 1) / divide);
          uint8_t thisPoint = vertical ? (dimension.y - y) : x + 1;
          if (thisPoint * piece >= value && (thisPoint - 1) * piece < value)
          {
            uint8_t scale = (int)(239 * (value - (thisPoint - 1) * piece) / piece + 16);
            //scale = scale > 255 ? 255 : scale;
            Color thisColor = knobPtr[i]->color.Blink_Color(active, Color(WHITE));
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          } 
          else if (thisPoint * piece <= value)
          {
            if (active) 
              MatrixOS::LED::SetColor(xy, knobPtr[i]->color);
            else 
              MatrixOS::LED::SetColor(xy, knobPtr[i]->color.ToLowBrightness());
          } 
          else
            MatrixOS::LED::SetColor(xy, Color(BLANK));
        } 
        else 
          MatrixOS::LED::SetColor(xy, Color(BLANK));
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    uint8_t i = vertical ? xy.x : xy.y;
    
    if(knobPtr[i] != nullptr && i < count){
      bool active = (i >= activePoint) && (i < activePoint + ENCODER_NUM);

      if (keyInfo->state == PRESSED)
      {
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
      };
      
      if (keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        float divide = vertical ? dimension.y : dimension.x;
        float piece = ((knobPtr[i]->max - knobPtr[i]->min + 1) / divide);
        float area = vertical ? ((float)dimension.y - xy.y - 0.5) : ((float)xy.x + 1 - 0.5);
        int16_t target = (int)(piece * area > 127 ? 127 : piece * area);
        knobPtr[i]->SetValue(target);
        MatrixOS::KnobCenter::Knob_Function(knobPtr[i]);
        // smoothList.emplace(i, target);
      }

      return true;
    }
    return false;
  }

  private:
  // void Smooth(uint16_t ms)
  // {
  //   uint8_t step = 127 / (ms / (1000 / Device::fps));
  //   for (auto it = smoothList.begin(); it != smoothList.end(); it++)
  //   {
  //     int8_t i = it->first;
  //     int16_t target = it->second;
  //     if (knobPtr[i]->byte2 < target) knobPtr[i]->byte2 = knobPtr[i]->byte2 + step > target ? target : knobPtr[i]->byte2 + step;
  //     if (knobPtr[i]->byte2 > target) knobPtr[i]->byte2 = knobPtr[i]->byte2 - step < target ? target : knobPtr[i]->byte2 - step;
  //     MatrixOS::KnobCenter::Knob_Function(knobPtr[i]);
  //     if (knobPtr[i]->byte2 == target) {
  //       smoothList.erase(it);
  //       break;
  //     }
  //   }
  // }

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