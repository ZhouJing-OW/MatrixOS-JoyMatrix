#pragma once
#include "MatrixOS.h"
#include "ui/UI.h"
#include <functional>
#include <vector>
#include <fstream>
#include <set>

namespace MatrixOS::KnobCenter
{                                       
  const Point knobOrigin = Point(8, 4);

  string appName;
  std::fstream fio;
  KnobConfig* knobAll;
  std::set<uint16_t> changedKnobs;

  bool channelMode;
  bool pageSelectMode;
  bool barDisplayMode;

  uint16_t knobCount;
  uint16_t knobsPerChannel;
  int8_t currentPage;
  int8_t prevPage;
  int8_t pageMax;
  int8_t extraPage;

  bool colorSetted;
  Color* color[16];
  Timer pageTimer;

  bool LoadKnobFiles(string name);
  void SaveKnob(KnobConfig& knob);
  void AddKnobBarTo(UI& ui);

  class KnobBar : public UIComponent 
  {
    const uint8_t lowBrightness = 8;

  public:
    std::vector<KnobConfig*> knobPtr;

    KnobBar()
    {
      knobPtr.reserve(ENCODER_NUM);
      for (uint8_t i = 0; i < ENCODER_NUM; i++)
        knobPtr.push_back(nullptr);
    }

    void Setup(vector<KnobConfig*> newKnobs)
    {
      uint8_t page = newKnobs.size() / ENCODER_NUM + (newKnobs.size() % ENCODER_NUM > 0);
      uint8_t size = page * ENCODER_NUM;
      knobPtr.clear();
      knobPtr.reserve(size);
      for (uint8_t i = 0; i < size ; i++)
      {
        if (i < newKnobs.size())
          knobPtr.push_back(newKnobs[i]);
        else
          knobPtr.push_back(nullptr);
      }
      // MLOGD("KnobBar", "Knob ptr: %d, %d, %d, %d, %d, %d, %d, %d", 
      // knobPtr[0], knobPtr[1], knobPtr[2], knobPtr[3], knobPtr[4], knobPtr[5], knobPtr[6], knobPtr[7]);
    }

    void DiableAll()
    {
      channelMode = false;
      for (uint8_t i = 0; i <ENCODER_NUM ; i++)
      {
        knobPtr[i] = nullptr;
      }
      Device::Encoder::DisableAll();
    }

    virtual Color GetColor() {
      if (knobPtr[0] != nullptr)
        return knobPtr[0]->color;
      else
        return COLOR_WHITE;
    }

    virtual Dimension GetSize() { return Dimension(ENCODER_NUM, 1); }

    virtual bool Render(Point origin) 
    {
      if (channelMode == true && !barDisplayMode)// enable page Select mode.
      {
        if (Device::KeyPad::Rocker() == true || !Device::AnalogInput::LRIsLonger(UI_POP_DELAY))
        {
          pageSelectMode = true;
          return PageSelect(origin);
        }
        else
          pageSelectMode = false;
      }
      else
        pageSelectMode = false;


      KnobConfig* dialKnob = Device::AnalogInput::GetDialPtr(); // enable bar display mode.
      uint8_t actEncoder = Device::Encoder::GetActEncoder();
      if (dialKnob != nullptr)
      {
        barDisplayMode = true;
        return BarDisplay(origin, dialKnob);
      }
      else if (actEncoder != 255 && Device::Encoder::GetEncoderPtr(actEncoder) != nullptr)
      {
        barDisplayMode = true;
        return BarDisplay(origin, Device::Encoder::GetEncoderPtr(actEncoder));
      }
      else
        barDisplayMode = false;


      
      for (uint8_t x = 0; x < ENCODER_NUM; x++) // Regular mode.
      {
        Point xy = origin + Point(x, 0);
        KnobConfig* knob = GetKnobPtr(x);
        if (Device::Encoder::GetEncoderPtr(x) != knob)
          Device::Encoder::Setup(knob, x);
        
        if (knob != nullptr && x < ENCODER_NUM)
        {
          int16_t val = knob->byte2;
          int16_t range = knob->max - knob->min + 1;
          int16_t halfRange = range / 2;

          if (knob->def == halfRange || knob->def == halfRange + range % 2 - 1)  // middleMode
          {
            if (val > halfRange + 1)  // right
            {
              Color thisColor = knob->color.Scale(val, halfRange, knob->max, lowBrightness);
              MatrixOS::LED::SetColor(xy, knob->lock ? thisColor : thisColor.Blink_Key(Device::KeyPad::fnState));
            }
            else if (val >= halfRange - 1 && val <= halfRange + 1)  // middle
            {
              Color thisColor = knob->color.Rotate(90).Scale(lowBrightness);
              MatrixOS::LED::SetColor(xy, knob->lock ? thisColor : thisColor.Blink_Key(Device::KeyPad::fnState));
            }
            else if (val < halfRange - 1)  // left
            {
              Color thisColor = knob->color.Invert().Scale(val, knob->min, halfRange, lowBrightness);
              MatrixOS::LED::SetColor(xy, knob->lock ? thisColor : thisColor.Blink_Key(Device::KeyPad::fnState));
            }
          }
          else  // Regular
          {
            Color thisColor = knob->color.Scale(val, knob->min, knob->max, lowBrightness);
            MatrixOS::LED::SetColor(xy, knob->lock ? thisColor : thisColor.Blink_Key(Device::KeyPad::fnState));
          }
        }
        else
          MatrixOS::LED::SetColor(xy, COLOR_BLANK);
      }
      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

      KnobConfig* knob = GetKnobPtr(xy.x);

      if (xy.x < ENCODER_NUM && knob != nullptr)
      {
        if(pageSelectMode) // Page select mode
        {
          if (keyInfo->state == PRESSED && xy.x <= pageMax + extraPage)
          {
            currentPage = xy.x;
            return true;
          }
          else 
            return false;
        }

        if (keyInfo->state == PRESSED) // Ragular Mode
        {
          Device::AnalogInput::UseDial(xy + knobOrigin, knob);
          if ((Device::KeyPad::fnState .active()) && knob->lock == false)
          {
            MatrixOS::Component::Knob_Setting(knob, false);
            return true;
          }
        }
        return true;
      }
      return false;
    }
  
  private:
    bool BarDisplay(Point origin, KnobConfig* displayKnob) {
      if(displayKnob->max - displayKnob->min + 1 <= ENCODER_NUM)
      { 
        for (uint8_t x = 0; x < ENCODER_NUM; x++) {
          Point xy = Point(x, 0) + origin;
          if (x <= displayKnob->byte2 - displayKnob->min)
            MatrixOS::LED::SetColor(xy, displayKnob->color);
          else if (x <= displayKnob->max - displayKnob->min)
            MatrixOS::LED::SetColor(xy, displayKnob->color.Scale(16));
          else
            MatrixOS::LED::SetColor(xy, COLOR_BLANK);
        }
        return true;
      }

      for (uint8_t x = 0; x < ENCODER_NUM; x++)
      {
        Point xy = Point(x, 0) + origin;
        Color color = displayKnob->color;

        int16_t full = (displayKnob->max - displayKnob->min + 1);
        int16_t half = (full / 2);
        float value = displayKnob->byte2;
        float divide = ENCODER_NUM;
        float piece = (full / divide);
        bool middleMode = (displayKnob->def == half || displayKnob->def == half + full % 2 - 1);
        uint8_t thisPoint = x + 1;
        if (middleMode)
        {
          if ((value >= half - 1) && (value <= half + 1))  // middle
          {
            if (x == ENCODER_NUM / 2 - 1)
              MatrixOS::LED::SetColor(xy, color.Invert().Scale(16));
            else if (x == ENCODER_NUM / 2)
              MatrixOS::LED::SetColor(xy, color.Scale(16));
            else
              MatrixOS::LED::SetColor(xy, COLOR_BLANK);
            continue;
          }
          else if (value > half + 1 && x < (ENCODER_NUM / 2))
          {
            MatrixOS::LED::SetColor(xy, COLOR_BLANK);
            continue;
          }
          else if (value < half - 1)  // left
          {
            if (x >= (ENCODER_NUM / 2))
            {
              MatrixOS::LED::SetColor(xy, COLOR_BLANK);
              continue;
            }
            thisPoint = ENCODER_NUM / 2 - x;
            value = half - value;
            color = color.Invert();
          }
        }

        if (thisPoint * piece >= value && (thisPoint - 1) * piece < value)
        {
          uint8_t scale = (int)((255 - lowBrightness) * (value - (thisPoint - 1) * piece) / piece + lowBrightness);
          MatrixOS::LED::SetColor(xy, color.Scale(scale));
        }
        else if (thisPoint * piece <= value)
          MatrixOS::LED::SetColor(xy, color);
        else
          MatrixOS::LED::SetColor(xy, COLOR_BLANK);
      }
      return true;
    }

    bool PageSelect(Point origin)
    {
      for(uint8_t x = 0; x < ENCODER_NUM; x++)
      {
        Point xy = Point(x, 0) + origin;
        if(x == currentPage)
          MatrixOS::LED::SetColor(xy, COLOR_WHITE);
        else if(x <= pageMax + extraPage)
        {
          Color thisColor = COLOR_KNOB_8PAGE[x];
          MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
        }
        else
          MatrixOS::LED::SetColor(xy, COLOR_BLANK);
      }
      return true;
    }

    KnobConfig* GetKnobPtr(uint16_t i)
    {
      if(channelMode == true)
      {
        if (currentPage <= pageMax)
        {
          i += currentPage * ENCODER_NUM;
          if ( i < knobsPerChannel)
          {
            uint8_t ch = MatrixOS::UserVar::global_channel;
            i += (ch * knobsPerChannel);
            return &knobAll[i];
          }
          else 
            return nullptr;
        }
        else
        {
          i += (currentPage - (pageMax + 1)) * ENCODER_NUM;
          if (i < knobPtr.size())
            return knobPtr[i];
          else
            return nullptr;
        }
      }
      else
        return knobPtr[i];
    }

  } knobBar;

}
