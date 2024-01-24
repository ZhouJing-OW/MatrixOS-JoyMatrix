#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class KnobButton : public UIComponent {
 public:
  std::vector<KnobConfig> knob;
  std::vector<uint16_t> knobID;
  Dimension dimension;
  uint8_t count;

  KnobButton(){};
  KnobButton(Dimension dimension, uint8_t count) {
    Setup(dimension, count);
  };

  void Setup(Dimension dimension, uint8_t count){
    this->dimension = dimension;
    this->count = count;
    for(uint8_t n = 0; n < count; n++){
      KnobConfig tempKnob;
      knob.push_back(tempKnob);
      knobID.push_back(0xFFFF);
    }
  }

  bool Disable(uint8_t n) {
    if (n < count) {
      this->knob[n].byte1 = -1;
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

  virtual Color GetColor() { return knob[0].color; }
  virtual Dimension GetSize() { return dimension; }

  virtual bool Render(Point origin) {

    for (uint8_t x = 0; x < dimension.x; x++) 
    { 
      for(uint8_t y = 0; y < dimension.y; y++) 
      {
        Point xy = origin + Point(x, 0);
        uint8_t i = y * dimension.x + x;
        int8_t val = knob[i].byte2;
        Color thisColor = knob[i].color;
        
        uint8_t LowLight = 16;
        float ratio = (256 - LowLight) / 2;
        int8_t range = knob[i].max - knob[i].min;
        int8_t halfRange = range / 2;

        if (knob[i].enable == true && i < count) // knob[i].byte1 < 0 = disable
        { 
          if (knob[i].def > halfRange - 1 && knob[i].def < halfRange + 1)
          {
              float hue;
              float s;
              float v;
            if (val >= halfRange + 2)
            {
              uint8_t halfVal = val - halfRange < 0 ? 0 : val - halfRange;
              uint8_t scale = (halfVal * ratio / halfRange + LowLight) > 255 ? 255 : (halfVal * ratio / halfRange + LowLight);
              MatrixOS::LED::SetColor(xy, knob[i].lock ? thisColor.Scale(scale) : thisColor.Scale(scale).Blink(Device::KeyPad::fnState));
            }
            else if (val >= halfRange - 2 && val <= halfRange + 2)
            {
              Color::RgbToHsv(thisColor, &hue, &s, &v);
              Color tempColor = Color::HsvToRgb(hue, 0.5, 1);
              MatrixOS::LED::SetColor(xy, knob[i].lock ? tempColor.Scale(LowLight) : tempColor.Scale(LowLight).Blink(Device::KeyPad::fnState));
            }
            else if (val < halfRange - 2)
            {
              Color::RgbToHsv(thisColor, &hue, &s, &v);
              if(0.5 - hue > 0)
                hue = 0.5 - hue;
              else
                hue = 1.5 - hue;
              Color tempColor = Color::HsvToRgb(hue, s, v);
              
              uint8_t scale = (((halfRange - val) * ratio / halfRange + LowLight) > 255 ? 255 : ((halfRange - val) * ratio / halfRange + LowLight));
              MatrixOS::LED::SetColor(xy, knob[i].lock ? tempColor.Scale(scale) : tempColor.Scale(scale).Blink(Device::KeyPad::fnState));
            }
          }
          else
          {
            uint8_t scale = (val * ratio / range + LowLight) > 255 ? 255 : (val * ratio / range + LowLight);
            MatrixOS::LED::SetColor(xy, knob[i].lock ? thisColor.Scale(scale) : thisColor.Scale(scale).Blink(Device::KeyPad::fnState));
          }
        } else {
          MatrixOS::LED::SetColor(xy, COLOR_BLANK);
        }
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

    uint8_t i = xy.y * dimension.x + xy.x;
    if (i < count) {
      if (keyInfo->state == PRESSED && knob[i].enable == true){
        if((Device::KeyPad::fnState == ACTIVATED || Device::KeyPad::fnState == HOLD) && knob[i].lock == false) {
          MatrixOS::Component::Knob_Setting(&knob[i], false);
          return true;
        }
      }

      if (keyInfo->state == HOLD && knob[i].enable == true)
      {
        knob[i].byte2 = knob[i].def;
        MatrixOS::Component::Knob_Function(&knob[i]);
      };
      return true;
    }
    return false;
  }
};
