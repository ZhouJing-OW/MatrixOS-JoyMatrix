#pragma once
#include "MatrixOS.h"
#include "KnobButton.h"

class MixerFader : public UIComponent {
 public:
  Dimension dimension;
  std::vector<KnobConfig*> knobPt;  // When used as a knob bar, this pointer points to the encoder. 
  std::vector<KnobConfig> knob;     // When used as a knob bar, The encoder pointer points to the container
  std::vector<uint16_t> knobID;
  std::function<void()> callback;
  std::unordered_map<int8_t, int8_t> smoothList;
  bool enableActivePoint;
  bool vertical;
  int8_t activePoint = -ENCODER_NUM;
  uint8_t count;
  Point position = Point(0, 0);

  MixerFader(){};
  MixerFader(Dimension dimension, uint8_t count,bool enableActivePoint = true,bool vertical = true, std::function<void()> callback = nullptr){
    Setup(dimension, count, enableActivePoint, vertical, callback);
  }

  void Setup(Dimension dimension, uint8_t count, bool enableActivePoint = false, bool vertical = true, std::function<void()> callback = nullptr){
    this->dimension = dimension;
    this->count = count;
    this->callback = callback;
    this->enableActivePoint = enableActivePoint;
    this->vertical = vertical;
    for (uint8_t n = 0; n < count; n++)
    {
      KnobConfig tempKnob;
      knob.push_back(tempKnob);
      knobID.push_back(0xFFFF);
    }
    for(uint8_t n = 0; n < count; n++){
      knobPt.push_back(&knob[n]);
    }
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

    for (uint8_t y = 0; y < dimension.y; y++){ 
      for (uint8_t x = 0; x < dimension.x; x++){
        Point xy = origin + Point(x, y);
        uint8_t i = vertical ? x : y;
        bool active = enableActivePoint && (i >= activePoint) && (i < activePoint + ENCODER_NUM);

        if(i < count){
          float value = knobPt[i]->byte2;
          float divide = vertical ? dimension.y : dimension.x;
          float piece = ((knobPt[i]->max - knobPt[i]->min + 1) / divide);
          uint8_t thisPoint = vertical ? (dimension.y - y) : x + 1;
          if (thisPoint * piece >= value && (thisPoint - 1) * piece < value)
          {
            uint8_t scale = (int)(239 * (value - (thisPoint - 1) * piece) / piece + 16);
            //scale = scale > 255 ? 255 : scale;
            MatrixOS::LED::SetColor(xy, knobPt[i]->color.Blink(active, COLOR_WHITE).Scale(scale));
          } 
          else if (thisPoint * piece <= value)
          {
            if (active) 
              MatrixOS::LED::SetColor(xy, knobPt[i]->color);
            else 
              MatrixOS::LED::SetColor(xy, knobPt[i]->color.ToLowBrightness());
          } else {
            MatrixOS::LED::SetColor(xy, COLOR_BLANK);
          }
        } 
        else MatrixOS::LED::SetColor(xy, COLOR_BLANK);
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    uint8_t i = vertical ? xy.x : xy.y;
    
    if(i < count){
      bool active = (i >= activePoint) && (i < activePoint + ENCODER_NUM);
      if ((enableActivePoint && active) || !enableActivePoint)
      {
        if (keyInfo->state == PRESSED && knobPt[i]->enable == true)
        {
          Device::AnalogInput::UseDial(xy + position, knobPt[i]);
        };
        
        if (keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          float divide = vertical ? dimension.y : dimension.x;
          float piece = ((knobPt[i]->max - knobPt[i]->min + 1) / divide);
          float area = vertical ? ((float)dimension.y - xy.y - 0.5) : ((float)xy.x + 1 - 0.5);
          uint8_t target = (int)(piece * area > 127 ? 127 : piece * area);
          smoothList.emplace(i, target);
          // knobPt[i]->byte2 = (int)(piece * area > 127 ? 127 : piece * area);
          // MatrixOS::Component::Knob_Function(&knob[i]);
        }
      } 
      
      if (enableActivePoint && !active)
      {
        if (keyInfo->state == PRESSED && knobPt[i]->enable == true)
        {
          activePoint = i - i % ENCODER_NUM;
          Callback();
          
          Device::AnalogInput::UseDial(xy + position, knobPt[i]);
        };

        if (keyInfo->state == RELEASED)
        {
          
        }
      }
      return true;
    }
    return false;
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