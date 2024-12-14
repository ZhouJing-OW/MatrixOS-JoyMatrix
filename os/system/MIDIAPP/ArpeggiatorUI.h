#pragma once
#include "MatrixOS.h"
#include "NodeUI.h"

namespace MatrixOS::MidiCenter
{
  class ArpeggiatorUI : public NodeUI
  {
    public:
    Arpeggiator* arpeggiator;
    virtual void SetNull() { arpeggiator = nullptr; }

    ArpeggiatorUI () {
      SetKnobPtr();
      channel = MatrixOS::UserVar::global_channel;
      channelPrv = channel;
      if(CheckNodeChange(arpeggiator, NODE_ARP))
        VarGet();
      KnobInit();
    }

    virtual bool SetKnobPtr() { 
      knobPtr = {&type, &rate, &octaveRange, &noteRepeat, &patternLength, &chance, &gate, &velDecay};
      return true;
    }

    virtual Color GetColor() { return Color(ORANGE); }
    virtual Dimension GetSize() { return dimension; }
    
    const Point labelPos        = Point(4, 0);
    const Point settingPos      = Point(0, 1);
    const Color knobColor[8]    = { Color(TURQUOISE), Color(CYAN),      Color(GREEN),     Color(LAWN),      Color(YELLOW),    Color(GOLD),      Color(ORANGE),    Color(RED) };
    const char labelName[8][8]  = { "Type",           "Rate",           "Octave",         "Repeat",         "Pattern",        "Chance",         "Gate",           "Decay" };
    const Color labelColor[8]   = { Color(LAWN),      Color(LAWN),      Color(LAWN),      Color(LAWN),      Color(YELLOW),    Color(YELLOW),    Color(YELLOW),    Color(YELLOW) };
    const char rateName[16][8]  = { "1/16",           "1/12",           "1/8",            "1/6",       
                                    "1/4",            "1/3",            "1/2", 
                                    "1Beat",          "2Beat",          "3Beat",        
                                    "4Beat",          "6Beat",          "8Beat",          "12Beat",         "16Beat",         "32Beat" };
    const Color rateColor[16]   = { Color(ORANGE),    Color(ORANGE),    Color(ORANGE),    Color(ORANGE), 
                                    Color(GOLD),      Color(GOLD),      Color(GOLD), 
                                    Color(YELLOW),    Color(YELLOW),    Color(YELLOW),   
                                    Color(LAWN),      Color(LAWN),      Color(LAWN),      Color(LAWN),      Color(LAWN),      Color(LAWN) };
    const char gateName[16][8]  = { "5%",             "10%",            "15%",            "20%",            "30%",            "40%",          
                                    "50%",            "60%",            "70%",            "80%",            "90%",            "100%", 
                                    "150%",           "200%",           "300%",           "400%" };
    const Color gateColor[16]   = { Color(YELLOW),    Color(YELLOW),    Color(YELLOW),    Color(YELLOW),    Color(YELLOW),    Color(YELLOW),
                                    Color(GOLD),      Color(GOLD),      Color(GOLD),      Color(GOLD),      Color(GOLD),      Color(GOLD),
                                    Color(ORANGE),    Color(ORANGE),    Color(ORANGE),    Color(ORANGE) };     
    const char typeName[12][16] = { "UP",             "Down",           "Converge",       "Diverge",        "Pink Up",        "Pink Down",       
                                    "Thumb Up",       "Thumb Down",     "Random",         "Random Other",   "Random Once",    "By Order"};
    const Color typeColor[12]   = { Color(YELLOW),    Color(YELLOW),    Color(YELLOW),    Color(YELLOW),    Color(YELLOW),    Color(YELLOW),   
                                    Color(YELLOW),    Color(YELLOW),    Color(LAWN),      Color(LAWN),      Color(LAWN),      Color(ORANGE)};
    const uint8_t decayNum[16]  = { 0, 1, 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 40, 48 };

    virtual bool Render(Point origin)
    {
      CheckNodeChange(arpeggiator, NODE_ARP);

      Color switchColor = Color(WHITE);
      MatrixOS::LED::SetColor(origin + Point(15, 0), switchColor.DimIfNot(arpeggiator != nullptr));
      LabelRender(origin + labelPos);

      if (arpeggiator == nullptr) return true;

      VarSet();
      Color configColor = arpeggiator->activeLabel == 8 ? Color(WHITE) : COLOR_HIGH_S[arpeggiator->configNum];
      MatrixOS::LED::SetColor(origin + Point(14, 0), configColor);
      // Color timeSyncColor = Color(BLUE);
      // MatrixOS::LED::SetColor(origin + Point(2, 0), timeSyncColor.DimIfNot(arpeggiator->config->timeSync));
      switch (arpeggiator->activeLabel)
      { 
        case 0: TypeRender      (origin + settingPos); break;
        case 1: RateRender      (origin + settingPos); break;
        case 2: 
        case 3: 
          OctaveRender(origin + settingPos);
          RepeatRender(origin + settingPos + Point(8, 0));
          break;
        case 4: PatternRender   (origin + settingPos); break;
        case 5: ChanceRender    (origin + settingPos); break;
        case 6: GateRender      (origin + settingPos); break;
        case 7: DecayRender     (origin + settingPos); break;
        case 8: ConfigRender    (origin + settingPos); break;
      }
      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
    {
      if(xy == Point(15, 0))
      {
        if (keyInfo->state == RELEASED && keyInfo->hold == false) {
          if (arpeggiator == nullptr) On();
          else Off();
          return true;
        }
        if (keyInfo->state == HOLD) {
          MatrixOS::UIInterface::TextScroll("ARP ON/OFF", Color(ORANGE));
          return true;
        }
      }

      if (arpeggiator == nullptr) 
        return false;

      // if(xy == Point(2 , 0))
      // {
      //   if (keyInfo->state == RELEASED && keyInfo->hold == false) {
      //     arpeggiator->config->timeSync = !arpeggiator->config->timeSync;
      //     return true;
      //   }
      //   if (keyInfo->state == HOLD) {
      //     MatrixOS::UIInterface::TextScroll("Sync", Color(ORANGE));
      //     return true;
      //   }
      // }

      if(xy == Point(14, 0))
      {
        if (keyInfo->state == RELEASED && keyInfo->hold == false) { 
          arpeggiator->activeLabel = 8; 
          return true; 
        }
        if (keyInfo->state == HOLD) {
          MatrixOS::UIInterface::TextScroll("Config Select", COLOR_HIGH_S[arpeggiator->configNum]);
          return true;
        }
      }
      
      if (xy.y == 0 && xy.x > 3 && xy.x < 12)
        return LableKeyEvent(xy, labelPos, keyInfo);
      if (xy.y == 1)
      {
        switch(arpeggiator->activeLabel)
        {
          case 0: return TypeKeyEvent     (xy, settingPos, keyInfo);
          case 1: return RateKeyEvent     (xy, settingPos, keyInfo);
          case 2: 
          case 3: 
            if (xy.x < 8) return OctaveKeyEvent(xy, settingPos, keyInfo);
            else return RepeatKeyEvent(xy, settingPos + Point(8, 0), keyInfo);
          case 4: return PatternKeyEvent  (xy, settingPos, keyInfo);
          case 5: return ChanceKeyEvent   (xy, settingPos, keyInfo);
          case 6: return GateKeyEvent     (xy, settingPos, keyInfo);
          case 7: return DecayKeyEvent    (xy, settingPos, keyInfo);
          case 8: return ConfigKeyEvent   (xy, settingPos, keyInfo);
        }
      }
      return false;
    }

    void LabelRender(Point origin)
    {
      Color activeColor = Color(WHITE);
      for(uint8_t x = 0; x < 8; x++)
      {
        Point xy = origin + Point(x, 0);
        Color thisColor = labelColor[x];
        MatrixOS::LED::SetColor(xy, thisColor.DimIfNot(arpeggiator != nullptr));
        if (arpeggiator == nullptr) continue;
        
        if (x == arpeggiator->activeLabel)
          MatrixOS::LED::SetColor(xy, activeColor);
        if ((arpeggiator->activeLabel == 2 && x == 3) || (arpeggiator->activeLabel == 3 && x == 2))
          MatrixOS::LED::SetColor(xy, activeColor.Scale(127));
      }
    }

    bool LableKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;

      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        MatrixOS::KnobCenter::SetPage(4);
        arpeggiator->activeLabel = ui.x;
        arpeggiator->lastLabel = ui.x;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(labelName[ui.x], knobPtr[ui.x]->color);
        return true;
      }
      return false;
    }

    void TypeRender(Point origin)
    {
      Color forbackColor = Color(BLUE);
      Color selectColor = arpeggiator->config->forBackward ? forbackColor : knobColor[0];
      Color repeatColor = Color(GOLD);
      for(uint8_t x = 0; x < 16; x++)
      {
        Point xy = origin + Point(x, 0);
        if ( x == 0)
          MatrixOS::LED::SetColor(xy, forbackColor.DimIfNot(arpeggiator->config->forBackward));
        else if (x == 1)
          MatrixOS::LED::SetColor(xy, repeatColor.DimIfNot(arpeggiator->config->forBackward && arpeggiator->config->repeatEnds));
        else if (x > 3)
        {
          uint8_t i = x - 4;
          Color thisColor = typeColor[i];
          if(i == arpeggiator->config->type)
            MatrixOS::LED::SetColor(xy, selectColor);
          else
            MatrixOS::LED::SetColor(xy, thisColor.DimIfNot());
        }
      }
    }

    bool TypeKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if (ui.x == 0)
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          arpeggiator->config->forBackward = !arpeggiator->config->forBackward;
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("For-Backward", Color(LAWN));
          return true;
        }
      }
      if (ui.x == 1)
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false && arpeggiator->config->forBackward)
        {
          arpeggiator->config->repeatEnds = !arpeggiator->config->repeatEnds;
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("Repeat-Ends", Color(GREEN));
          return true;
        }
      }
      if (ui.x > 3)
      {
        if(keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll(typeName[ui.x - 4], typeColor[ui.x - 4]);
          return true;
        }

        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          type.SetValue(ui.x - 4);
          return true;
        }
      }
      return false;
    }

    void RateRender(Point origin)
    {
      for(uint8_t x = 0; x < 16; x++)
      {
        Point xy = origin + Point(x, 0);
        Color thisColor = rateColor[x];
        if (x == rate.Value())
        {
          if(arpeggiator->arpInterval >= 2000.0 / Device::fps)
          {
            if(arpeggiator->inputList.empty())
              MatrixOS::LED::SetColor(xy, thisColor.Blink_Interval(arpeggiator->arpInterval, knobColor[1], arpeggiator->arpTimer));
            else
              MatrixOS::LED::SetColor(xy, thisColor);
          }
          else
            MatrixOS::LED::SetColor(xy, Color(WHITE));
        }
        else
          MatrixOS::LED::SetColor(xy, thisColor.DimIfNot());
      }
    }

    bool RateKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        rate.SetValue(ui.x);
        return true;
      }

      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(rateName[ui.x], rateColor[ui.x]);
        return true;
      }
      return false;
    }

    void OctaveRender(Point origin)
    {
      Color thisColor = knobColor[2];
      for(uint8_t x = 0; x < 8; x++)
      {
        Point xy = origin + Point(x, 0);
        if(x < 6)
        {
          if (x == arpeggiator->currentOctave && !arpeggiator->inputList.empty())
            MatrixOS::LED::SetColor(xy, Color(ORANGE));
          else if(x < octaveRange.Value())
            MatrixOS::LED::SetColor(xy, thisColor);
          else
            MatrixOS::LED::SetColor(xy, thisColor.DimIfNot());
        }
        else
          MatrixOS::LED::SetColor(xy, Color(BLANK));
      }
    }

    bool OctaveKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if (ui.x < 6)
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          octaveRange.SetValue(ui.x + 1);
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll(std::to_string(ui.x + 1), Color(RED));
          return true;
        }
      }
      return false;
    }

    void RepeatRender(Point origin)
    {
      Color thisColor = knobColor[3];
      for(uint8_t x = 0; x < 8; x++)
      {
        Point xy = origin + Point(x, 0);
        if(x == arpeggiator->currentRepeat && !arpeggiator->inputList.empty())
          MatrixOS::LED::SetColor(xy, Color(ORANGE));
        else if(x < noteRepeat.Value())
          MatrixOS::LED::SetColor(xy, thisColor);
        else
          MatrixOS::LED::SetColor(xy, thisColor.DimIfNot());
      }
    }

    bool RepeatKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        noteRepeat.SetValue(ui.x + 1);
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(std::to_string(ui.x + 1) + "x", Color(ORANGE));
        return true;
      }
      return false;
    }

    void PatternRender(Point origin)
    {
      Color thisColor = Color(ORANGE);
      Color backColor = Color(BLUE);
      Color skipColor = Color(RED);
      Color lengthColor = arpeggiator->config->skip ? Color(RED) : knobColor[4]; 
      for(uint8_t x = 0; x < 16; x++)
      {
        Point xy = origin + Point(x, 0);
        if (x == 15)
          MatrixOS::LED::SetColor(xy, skipColor.DimIfNot(arpeggiator->config->skip));
        else if (x > 3 && x < 12)
        {
          uint8_t i = x - 4;
          if(i == arpeggiator->currentStep && !arpeggiator->inputList.empty())
          MatrixOS::LED::SetColor(xy, Color(WHITE));
          else if(i < arpeggiator->config->patternLength)
          {
            bool active = bitRead(arpeggiator->config->pattern, i);
            uint8_t scale = arpeggiator->config->velocity[i];
            MatrixOS::LED::SetColor(xy, active ? thisColor.Scale(scale, 0, 127, 16) : lengthColor.DimIfNot());
          }
          else
            MatrixOS::LED::SetColor(xy, backColor.DimIfNot());
        }
        else if (x == 0)
          MatrixOS::LED::SetColor(xy, backColor.DimIfNot());
        else if (x == 1)
          MatrixOS::LED::SetColor(xy, lengthColor.DimIfNot());
        else
          MatrixOS::LED::SetColor(xy, Color(BLANK));
      }
    }

    bool PatternKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if (ui.x == 0)
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          patternLength.SetValue(patternLength.Value() - 1 > 1 ? patternLength.Value() - 1 : 1);
          return true;
        }
        if (keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("-Step", Color(LAWN));
          return true;
        }
      }
      if (ui.x == 1)
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          patternLength.SetValue(patternLength.Value() + 1 < 9 ? patternLength.Value() + 1 : 8);
          return true;
        }
        if (keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("+Step", Color(LAWN));
          return true;
        }
      }
      if (ui.x > 3 && ui.x < arpeggiator->config->patternLength + 4)
      {
        if(keyInfo->state == PRESSED)
        {
          if (bitRead(arpeggiator->config->pattern, ui.x - 4))
            Device::AnalogInput::UseDial(xy, &velocity[ui.x - 4]);
          return true;
        }

        if(ui.x != 4 && keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          arpeggiator->config->pattern ^= (1 << (ui.x - 4));
          return true;
        }
      }
      if (ui.x > 4 && ui.x >= arpeggiator->config->patternLength + 4 && ui.x < 12)
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          patternLength.SetValue(ui.x - 3);
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("Length", Color(LAWN));
          return true;
        }
      }
      if(ui.x == 15)
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          arpeggiator->config->skip = !arpeggiator->config->skip;
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("Skip", Color(RED));
          return true;
        }
      }
      
      return false;
    }

    void ChanceRender(Point origin)
    {
      Color thisColor = knobColor[5];
      Color backColor = arpeggiator->config->skip ? Color(RED) : Color(BLUE);
      Color skipColor = Color(RED);
      Color blinkColor = knobColor[2];
      for(uint8_t x = 0; x < 16; x++)
      {
        Point xy = origin + Point(x, 0);
        if(x == 0)
          MatrixOS::LED::SetColor(xy, blinkColor.DimIfNot());
        else if (x == 1)
          MatrixOS::LED::SetColor(xy, blinkColor);
        else if(x > 2 && x < 13)
        {
          uint8_t i = x - 3;
          if(i >= chance.Value() / 10)
            MatrixOS::LED::SetColor(xy, backColor.DimIfNot().Blink_Color((chance.Value() - 1) % 10 == i, blinkColor));
          else
            MatrixOS::LED::SetColor(xy, thisColor.Blink_Color((chance.Value() - 1) % 10 == i, blinkColor));
        }
        else if(x == 15)
          MatrixOS::LED::SetColor(xy, skipColor.DimIfNot(arpeggiator->config->skip));
        else
          MatrixOS::LED::SetColor(xy, Color(BLANK));
      }
    }

    bool ChanceKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if (ui.x == 0)
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          chance.SetValue(chance.Value() - 1 > 5 ? chance.Value() - 1 : 5);
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("-1%", Color(ORANGE));
          return true;
        }
      }
      if (ui.x == 1)
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          chance.SetValue(chance.Value() + 1 < 100 ? chance.Value() + 1 : 100);
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("+1%", Color(ORANGE));
          return true;
        }
      }
      if (ui.x > 2 && xy.x < 13)
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          chance.SetValue((ui.x - 3) * 10 + (chance.Value() % 10 == 0 ? 10 : chance.Value() % 10));
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          string cs = std::to_string(chance.Value()) + "%";
          MatrixOS::UIInterface::TextScroll(cs, Color(ORANGE));
          return true;
        }
      }
      if(ui.x == 15)
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          arpeggiator->config->skip =!arpeggiator->config->skip;
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("Skip", Color(RED));
          return true;
        }
      }
      return false;
    }

    void GateRender(Point origin)
    {
      for(uint8_t x = 0; x < 16; x++)
      {
        Point xy = origin + Point(x, 0);
        Color thisColor = gateColor[x];
        if (x == gate.Value())
          MatrixOS::LED::SetColor(xy,knobColor[6]);
        else if (x < gate.Value())
          MatrixOS::LED::SetColor(xy, gateColor[gate.Value()]);
        else
          MatrixOS::LED::SetColor(xy, thisColor.DimIfNot());
      }
    }

    bool GateKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        gate.SetValue(ui.x);
        return true;
      }

      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(gateName[ui.x], gateColor[ui.x]);
        return true;
      }
      return false;
    }

    void DecayRender(Point origin)
    {
      Color thisColor = knobColor[7];
      Color backColor = Color(BLUE);
      Color velColor = Color(GREEN);
      for(uint8_t x = 0; x < 16; x++)
      {
        Point xy = origin + Point(x, 0);
        if(arpeggiator->inputList.empty())
        {
          if (x == 0)
            MatrixOS::LED::SetColor(xy, thisColor.DimIfNot());
          else if (decayNum[x] < velDecay.Value())
            MatrixOS::LED::SetColor(xy, thisColor.DimIfNot());
          else if (decayNum[x - 1] < velDecay.Value() && decayNum[x] >= velDecay.Value())
          {
            uint8_t scale = (int)((255 - 16) * (velDecay.Value() - decayNum[x - 1]) / (decayNum[x] - decayNum[x - 1]) + 16);
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          }
          else
            MatrixOS::LED::SetColor(xy, backColor.DimIfNot());
        }
        else
        {
          if(x == 0 && arpeggiator->decayNow >= 127)
            MatrixOS::LED::SetColor(xy, backColor.DimIfNot());
          else if ( x * 8 < 127 - arpeggiator->decayNow)
          {
            MatrixOS::LED::SetColor(xy, velColor);
          }
          else if ((x - 1) * 8 < 127 - arpeggiator->decayNow && x * 8 >= 127 - arpeggiator->decayNow)
          {
            uint8_t scale = (int)((255 - 16) * (127 - arpeggiator->decayNow - (x - 1) * 8) / 8 + 16);
            MatrixOS::LED::SetColor(xy, velColor.Scale(scale));
          }
          else
            MatrixOS::LED::SetColor(xy, backColor.DimIfNot());
          
          if (decayNum[x - 1] < velDecay.Value() && decayNum[x] >= velDecay.Value())
          {
            uint8_t scale = (int)((255 - 16) * (velDecay.Value() - decayNum[x - 1]) / (decayNum[x] - decayNum[x - 1]) + 16);
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          }
        }
      }
    }

    bool DecayKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        velDecay.SetValue(decayNum[ui.x]);
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("-" + std::to_string(decayNum[ui.x]), Color(RED));
        return true;
      }
      return false;
    }

    void ConfigRender(Point origin)
    {
      for(uint8_t x = 0; x < 16; x++)
      {
        Point xy = origin + Point(x, 0);
        Color thisColor = COLOR_HIGH_S[x];
        if (arpeggiator->configNum == x)
          MatrixOS::LED::SetColor(xy, thisColor);
        else
          MatrixOS::LED::SetColor(xy, thisColor.DimIfNot());
      }
    }
    
    bool ConfigKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        arpeggiator->SetActiveConfig(ui.x);
        VarGet();
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("Config " + std::to_string(ui.x + 1), COLOR_HIGH_S[ui.x]);
        return true;
      }
      return false;
    }
    
    virtual void VarGet()
    {
      if(arpeggiator == nullptr) return;

      for(uint8_t i = 0; i < 8; i++)
      {
        velocity[i].SetPtr(&arpeggiator->config->velocity[i]);
        knobPtr[i]->SetPtr(arpeggiator->configPtr[i]);
      }
    }

    virtual void VarSet()
    {
      if(arpeggiator == nullptr) return;

      uint8_t activeKnob = Device::Encoder::GetActEncoder();
      KnobConfig* ptr = Device::Encoder::GetEncoderPtr(activeKnob);
      if (ptr != nullptr && ptr == knobPtr[activeKnob])
        arpeggiator->activeLabel = arpeggiator->lastLabel = activeKnob;

      ptr = Device::AnalogInput::GetDialPtr();
      if(ptr != nullptr) {
        for(uint8_t i = 0; i < 8; i++) {
          if (ptr == knobPtr[i]) {
            arpeggiator->activeLabel = arpeggiator->lastLabel = i;
            break;
          }
        }
      }
    }

    KnobConfig type;                        // 0 - 11
    KnobConfig rate;                        // 1/16, 1/12, 1/8, 1/6, 1/4, 1/3, 1/2, 1, 2, 3, 4, 6, 8, 12, 16, 32
    KnobConfig octaveRange;                 // 1 - 6 
    KnobConfig noteRepeat;                  // 1 - 8
    KnobConfig patternLength;               // 1 - 8
    KnobConfig chance;                      // 5 - 100
    KnobConfig gate;                        // 5% , 10%, 15%, 20%, 30%, 40%, def 50%, 60%, 70%, 80%, 90%, 100%, 150%, 200%, 300%, 400%
    KnobConfig velDecay;                    // 0 - 48
    KnobConfig velocity[8];

    void KnobInit()
    {
      for(uint8_t i = 0; i < 8; i++)
        velocity[i]     = {.lock = true, .data = {.varPtr = nullptr},   .min = 1,   .def = 127,  .color = Color(ORANGE)};

      type              = {.lock = true, .data = {.varPtr = nullptr},   .min = 0,   .max = 11,   .def = 0,   .color = knobColor[0]};
      rate              = {.lock = true, .data = {.varPtr = nullptr},   .min = 0,   .max = 15,   .def = 4,   .color = knobColor[1]};
      octaveRange       = {.lock = true, .data = {.varPtr = nullptr},   .min = 1,   .max = 6,    .def = 1,   .color = knobColor[2]};
      noteRepeat        = {.lock = true, .data = {.varPtr = nullptr},   .min = 1,   .max = 8,    .def = 1,   .color = knobColor[3]};
      patternLength     = {.lock = true, .data = {.varPtr = nullptr},   .min = 1,   .max = 8,    .def = 1,   .color = knobColor[4]};
      chance            = {.lock = true, .data = {.varPtr = nullptr},   .min = 5,   .max = 100,  .def = 100, .color = knobColor[5]};
      gate              = {.lock = true, .data = {.varPtr = nullptr},   .min = 0,   .max = 15,   .def = 6,   .color = knobColor[6]};
      velDecay          = {.lock = true, .data = {.varPtr = nullptr},   .min = 0,   .max = 48,   .def = 0,   .color = knobColor[7]};
    }
  };
}