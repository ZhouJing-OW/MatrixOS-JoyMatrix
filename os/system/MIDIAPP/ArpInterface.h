#pragma once
#include "MatrixOS.h"
#include "../os/ui/component/UIComponent.h"
#include "../os/ui/UIComponents.h"
#include "MidiCenter.h"

class ArpInterface : public UIComponent
{
  public:
  Dimension dimension = Dimension(16,2);
  KnobButton knob;
  uint8_t channel;
  uint8_t channelPrv;
  MatrixOS::MidiCenter::Arpeggiator* arpeggiator = nullptr;
  std::vector<KnobConfig*> knobPtr;
  ArpConfig* arpConfig = nullptr;

  ArpInterface(ArpConfig* arpConfig = nullptr) {
    knobPtr = {&rate, &patternLength, &chance, &gate, &type, &octaveRange, &noteRepeat, &velDecay};
    KnobInit();
    knob.Setup(Dimension(8, 1), 8);
    knob.SetKnobs(knobPtr);
    if(arpConfig != nullptr)
      this->arpConfig = arpConfig;
  }

  void Setup(ArpConfig* arpConfig)
  {
    this->arpConfig = arpConfig;
  }

  bool FindArppegiator()
  {
    auto it = MatrixOS::MidiCenter::arpeggiators.find(channel);
    if (it != MatrixOS::MidiCenter::arpeggiators.end())
    {
      MLOGD("Arpeggiator", "channel: %d Actived", channel + 1);
      arpeggiator = it->second;
      VarGet();
      return true;
    }
    else
    {
      arpeggiator = nullptr;
      return false;
    }
  }

  void ArpOn()
  {
    channel = MatrixOS::UserVar::global_channel;
    MatrixOS::MidiCenter::NodeInsert(NODE_ARP, channel, arpConfig);
    MatrixOS::KnobCenter::AddExtraPage(knobPtr);
    MatrixOS::KnobCenter::SetPage(4);
  }

  void Arpoff()
  {
    channel = MatrixOS::UserVar::global_channel;
    MatrixOS::MidiCenter::NodeDelete(NODE_ARP, channel);
    MatrixOS::KnobCenter::DisableExtraPage();
    arpeggiator = nullptr;
  }

  virtual Color GetColor() { return COLOR_ORANGE; }
  virtual Dimension GetSize() { return dimension; }
  
  const Point labelPos = Point(4, 0);
  const Point settingPos = Point(0, 1);
  const Color knobColor[8]    = { COLOR_AZURE,    COLOR_CYAN,     COLOR_GREEN,    COLOR_LIME,     COLOR_YELLOW,   COLOR_GOLD,     COLOR_ORANGE,   COLOR_RED };
  const string labelName[8]   = { "Rate",         "Pattern",      "Chance",       "Gate",         "Type",         "Octave",       "Repeat",       "Decay" };
  const Color labelColor[8]   = { COLOR_GREEN,    COLOR_GREEN,    COLOR_GREEN,    COLOR_GREEN,    COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW };
  const string rateName[16]   = { "1/16",         "1/12",         "1/8",          "1/6",       
                                  "1/4",          "1/3",          "1/2", 
                                  "1Beat",        "2Beat",        "3Beat",        
                                  "4Beat",        "6Beat",        "8Beat",        "12Beat",       "16Beat",       "32Beat" };
  const Color rateColor[16]   = { COLOR_ORANGE,   COLOR_ORANGE,   COLOR_ORANGE,   COLOR_ORANGE, 
                                  COLOR_GOLD,     COLOR_GOLD,     COLOR_GOLD, 
                                  COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   
                                  COLOR_GREEN,    COLOR_GREEN,    COLOR_GREEN,    COLOR_GREEN,    COLOR_GREEN,    COLOR_GREEN };
  const string gateName[16]   = { "5%",           "10%",          "15%",          "20%",          "30%",          "40%",          
                                  "50%",          "60%",          "70%",          "80%",          "90%",          "100%", 
                                  "150%",         "200%",         "300%",         "400%" };
  const Color gateColor[16]   = { COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,
                                  COLOR_GOLD,     COLOR_GOLD,     COLOR_GOLD,     COLOR_GOLD,     COLOR_GOLD,     COLOR_GOLD,
                                  COLOR_ORANGE,   COLOR_ORANGE,   COLOR_ORANGE,   COLOR_ORANGE };     
  const string typeName[12]   = { "UP",           "Down",         "Converge",     "Diverge",      "PinkUp",       "PinkDown",       
                                  "ThumbUp",      "ThumbDown",    "Random",       "Random Other", "Random Once",  "By Order"};
  const Color typeColor[12]   = { COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   
                                  COLOR_YELLOW,   COLOR_YELLOW,   COLOR_GREEN,    COLOR_GREEN,    COLOR_GREEN,    COLOR_ORANGE};
  const uint8_t decayNum[16]  = { 0, 1, 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 40, 48 };

  virtual bool Render(Point origin)
  {
    channel = MatrixOS::UserVar::global_channel;
    if (channel != channelPrv || arpeggiator == nullptr)
    {
      channelPrv = channel;
      if (!FindArppegiator() && MatrixOS::KnobCenter::HaveExtraPage())
        MatrixOS::KnobCenter::DisableExtraPage();
      if (FindArppegiator() && !MatrixOS::KnobCenter::HaveExtraPage())
        MatrixOS::KnobCenter::AddExtraPage(knobPtr);
    }

    for(uint8_t x = 0; x < 16; x++)
    {
      MatrixOS::LED::SetColor(origin + Point(x, 0), COLOR_BLANK);
      MatrixOS::LED::SetColor(origin + Point(x, 1), COLOR_BLANK);
    }
    MatrixOS::LED::SetColor(origin + Point(0, 0), COLOR_RED);
    Color switchColor = COLOR_WHITE;
    MatrixOS::LED::SetColor(origin + Point(15, 0), switchColor.ToLowBrightness(arpeggiator != nullptr));

    LabelRender(origin + labelPos);

    if (arpeggiator == nullptr)
      return true;
    
    if (!MatrixOS::KnobCenter::HaveExtraPage())
      MatrixOS::KnobCenter::AddExtraPage(knobPtr);

    VarSync();
    switch (arpeggiator->activeLabel)
    { 
      case 0: RateRender      (origin + settingPos); break;
      case 1: PatternRender   (origin + settingPos); break;
      case 2: ChanceRender    (origin + settingPos); break;
      case 3: GateRender      (origin + settingPos); break;
      case 4: TypeRender      (origin + settingPos); break;
      case 5: 
      case 6: 
        OctaveRender(origin + settingPos);
        RepeatRender(origin + settingPos + Point(8, 0)); 
        break;
      case 7: DecayRender     (origin + settingPos); break;
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
  {
    if(xy == Point(15, 0) && arpConfig != nullptr)
    {
      if (keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        if (arpeggiator == nullptr)
          ArpOn();
        else
          Arpoff();
      }
      if (keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("ARP ON/OFF", COLOR_ORANGE);
        return true;
      }
    }

    if (arpeggiator == nullptr) 
      return false;
    
    if (xy.y == 0 && xy.x > 3 && xy.x < 12)
      return LableKeyEvent(xy, labelPos, keyInfo);
    if (xy.y == 1)
    {
      switch(arpeggiator->activeLabel)
      {
        case 0: return RateKeyEvent     (xy, settingPos, keyInfo);
        case 1: return PatternKeyEvent  (xy, settingPos, keyInfo);
        case 2: return ChanceKeyEvent   (xy, settingPos, keyInfo);
        case 3: return GateKeyEvent     (xy, settingPos, keyInfo);
        case 4: return TypeKeyEvent     (xy, settingPos, keyInfo);
        case 5: 
        case 6: 
          if (xy.x < 8)
            return OctaveKeyEvent(xy, settingPos, keyInfo);
          else
            return RepeatKeyEvent(xy, settingPos + Point(8, 0), keyInfo);
        case 7: return DecayKeyEvent    (xy, settingPos, keyInfo);
      }
    }
    return true;
  }

  void LabelRender(Point origin)
  {
    Color activeColor = COLOR_WHITE;
    for(uint8_t x = 0; x < 8; x++)
    {
      Point xy = origin + Point(x, 0);
      Color thisColor = labelColor[x];
      MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness(arpeggiator != nullptr));
      if (arpeggiator == nullptr) continue;
      
      if (x == arpeggiator->activeLabel)
        MatrixOS::LED::SetColor(xy, activeColor);
      if ((arpeggiator->activeLabel == 5 && x == 6) || (arpeggiator->activeLabel == 6 && x == 5))
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
      return true;
    }
    if(keyInfo->state == HOLD)
    {
      MatrixOS::UIInterface::TextScroll(labelName[ui.x], knobPtr[ui.x]->color);
      return true;
    }
    return false;
  }

  void RateRender(Point origin)
  {
    for(uint8_t x = 0; x < 16; x++)
    {
      Point xy = origin + Point(x, 0);
      Color thisColor = rateColor[x];
      if (x == rate.byte2)
      {
        if(arpeggiator->arpInterval >= 2000.0 / Device::fps)
        {
          if(arpeggiator->inputList.empty())
            MatrixOS::LED::SetColor(xy, thisColor.Blink_Interval(arpeggiator->arpInterval, knobColor[0], arpeggiator->arpTimer));
          else
            MatrixOS::LED::SetColor(xy, thisColor);
        }
        else
          MatrixOS::LED::SetColor(xy, COLOR_WHITE);
      }
      else
        MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
    }
  }

  bool RateKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
  {
    Point ui = xy - offset;
    if(keyInfo->state == RELEASED && keyInfo->hold == false)
    {
      rate.byte2 = ui.x;
      return true;
    }

    if(keyInfo->state == HOLD)
    {
      MatrixOS::UIInterface::TextScroll(rateName[ui.x], rateColor[ui.x]);
      return true;
    }
    return false;
  }

  void PatternRender(Point origin)
  {
    Color thisColor = knobColor[1];
    Color backColor = COLOR_BLUE;
    Color skipColor = COLOR_RED;
    Color lengthColor = skip ? COLOR_RED : COLOR_GREEN; 
    for(uint8_t x = 0; x < 16; x++)
    {
      Point xy = origin + Point(x, 0);
      if (x == 15)
        MatrixOS::LED::SetColor(xy, skipColor.ToLowBrightness(skip));
      else if (x > 3 && x < 12)
      {
        uint8_t i = x - 4;
        if(i == arpeggiator->currentStep && !arpeggiator->inputList.empty())
        MatrixOS::LED::SetColor(xy, COLOR_WHITE);
        else if(i < arpeggiator->config->patternLength)
        {
          bool active = bitRead(arpeggiator->config->pattern, i);
          uint8_t scale = arpeggiator->config->velocity[i];
          MatrixOS::LED::SetColor(xy, active ? thisColor.Scale(scale, 0, 127, 16) : lengthColor.ToLowBrightness());
        }
        else
          MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness());
      }
      else if (x == 0)
        MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness());
      else if (x == 1)
        MatrixOS::LED::SetColor(xy, lengthColor.ToLowBrightness());
      else
        MatrixOS::LED::SetColor(xy, COLOR_BLANK);
    }
  }

  bool PatternKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
  {
    Point ui = xy - offset;
    if (ui.x == 0)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        patternLength.byte2 = patternLength.byte2 - 1 > 1 ? patternLength.byte2 - 1 : 1;
        return true;
      }
      if (keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("-Step", COLOR_GREEN);
        return true;
      }
    }
    if (ui.x == 1)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        patternLength.byte2 = patternLength.byte2 + 1 < 9 ? patternLength.byte2 + 1 : 8;
        return true;
      }
      if (keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("+Step", COLOR_GREEN);
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
        patternLength.byte2 = ui.x - 3;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("Length", COLOR_GREEN);
        return true;
      }
    }
    if(ui.x == 15)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        skip = !skip;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("Skip", COLOR_RED);
        return true;
      }
    }
    
    return false;
  }

  void ChanceRender(Point origin)
  {
    Color thisColor = knobColor[1];
    Color backColor = skip ? COLOR_RED : COLOR_BLUE;
    Color skipColor = COLOR_RED;
    Color blinkColor = knobColor[2];
    for(uint8_t x = 0; x < 16; x++)
    {
      Point xy = origin + Point(x, 0);
      if(x == 0)
        MatrixOS::LED::SetColor(xy, blinkColor.ToLowBrightness());
      else if (x == 1)
        MatrixOS::LED::SetColor(xy, blinkColor);
      else if(x > 2 && x < 13)
      {
        uint8_t i = x - 3;
        if(i >= chance.byte2 / 10)
          MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness().Blink_Color((chance.byte2 - 1) % 10 == i, blinkColor));
        else
          MatrixOS::LED::SetColor(xy, thisColor.Blink_Color((chance.byte2 - 1) % 10 == i, blinkColor));
      }
      else if(x == 15)
        MatrixOS::LED::SetColor(xy, skipColor.ToLowBrightness(skip));
      else
        MatrixOS::LED::SetColor(xy, COLOR_BLANK);
    }
  }

  bool ChanceKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
  {
    Point ui = xy - offset;
    if (ui.x == 0)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        chance.byte2 = chance.byte2 - 1 > 5 ? chance.byte2 - 1 : 5;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("-1%", COLOR_ORANGE);
        return true;
      }
    }
    if (ui.x == 1)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        chance.byte2 = chance.byte2 + 1 < 100 ? chance.byte2 + 1 : 100;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("+1%", COLOR_ORANGE);
        return true;
      }
    }
    if (ui.x > 2 && xy.x < 13)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        chance.byte2 = (ui.x - 3) * 10 + (chance.byte2 % 10 == 0 ? 10 : chance.byte2 % 10);
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        string cs = std::to_string(chance.byte2) + "%";
        MatrixOS::UIInterface::TextScroll(cs, COLOR_ORANGE);
        return true;
      }
    }
    if(ui.x == 15)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        skip = !skip;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("Skip", COLOR_RED);
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
      if (x == gate.byte2)
        MatrixOS::LED::SetColor(xy,knobColor[3]);
      else if (x < gate.byte2)
        MatrixOS::LED::SetColor(xy, gateColor[gate.byte2]);
      else
        MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
    }
  }

  bool GateKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
  {
    Point ui = xy - offset;
    if(keyInfo->state == RELEASED && keyInfo->hold == false)
    {
      gate.byte2 = ui.x;
      return true;
    }

    if(keyInfo->state == HOLD)
    {
      MatrixOS::UIInterface::TextScroll(gateName[ui.x], gateColor[ui.x]);
      return true;
    }
    return false;
  }

  void TypeRender(Point origin)
  {
    Color forbackColor = knobColor[4];
    Color repeatColor = COLOR_GOLD;
    for(uint8_t x = 0; x < 16; x++)
    {
      Point xy = origin + Point(x, 0);
      if ( x == 0)
        MatrixOS::LED::SetColor(xy, forbackColor.ToLowBrightness(forBackward));
      else if (x == 1)
        MatrixOS::LED::SetColor(xy, repeatColor.ToLowBrightness(forBackward && repeatEnds));
      else if (x > 3)
      {
        uint8_t i = x - 4;
        Color thisColor = typeColor[i];
        if(i == arpeggiator->config->type)
          MatrixOS::LED::SetColor(xy, thisColor);
        else
          MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
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
        forBackward = !forBackward;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("For-Backward", COLOR_GREEN);
        return true;
      }
    }
    if (ui.x == 1)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false && forBackward)
      {
        repeatEnds = !repeatEnds;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll("Repeat-Ends", COLOR_LIME);
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
        type.byte2 = ui.x - 4;
        return true;
      }
    }
    return false;
  }

  void OctaveRender(Point origin)
  {
    Color thisColor = knobColor[5];
    for(uint8_t x = 0; x < 8; x++)
    {
      Point xy = origin + Point(x, 0);
      if(x < 6)
      {
        if (x == arpeggiator->currentOctave && !arpeggiator->inputList.empty())
          MatrixOS::LED::SetColor(xy, COLOR_ORANGE);
        else if(x < octaveRange.byte2)
          MatrixOS::LED::SetColor(xy, thisColor);
        else
          MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
      }
      else
        MatrixOS::LED::SetColor(xy, COLOR_BLANK);
    }
  }

  bool OctaveKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
  {
    Point ui = xy - offset;
    if (ui.x < 6)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        octaveRange.byte2 = ui.x + 1;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(std::to_string(ui.x + 1), COLOR_RED);
        return true;
      }
    }
    return false;
  }

  void RepeatRender(Point origin)
  {
    Color thisColor = knobColor[6];
    for(uint8_t x = 0; x < 8; x++)
    {
      Point xy = origin + Point(x, 0);
      if(x == arpeggiator->currentRepeat && !arpeggiator->inputList.empty())
        MatrixOS::LED::SetColor(xy, COLOR_ORANGE);
      else if(x < noteRepeat.byte2)
        MatrixOS::LED::SetColor(xy, thisColor);
      else
        MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
    }
  }

  bool RepeatKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
  {
    Point ui = xy - offset;
    if(keyInfo->state == RELEASED && keyInfo->hold == false)
    {
      noteRepeat.byte2 = ui.x + 1;
      return true;
    }
    if(keyInfo->state == HOLD)
    {
      MatrixOS::UIInterface::TextScroll(std::to_string(ui.x + 1) + "x", COLOR_ORANGE);
      return true;
    }
    return false;
  }

  void DecayRender(Point origin)
  {
    Color thisColor = knobColor[7];
    Color backColor = COLOR_BLUE;
    Color velColor = COLOR_LIME;
    for(uint8_t x = 0; x < 16; x++)
    {
      Point xy = origin + Point(x, 0);
      if(arpeggiator->inputList.empty())
      {
        if (x == 0)
          MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
        else if (decayNum[x] < velDecay.byte2)
          MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
        else if (decayNum[x - 1] < velDecay.byte2 && decayNum[x] >= velDecay.byte2)
        {
          uint8_t scale = (int)((255 - 16) * (velDecay.byte2 - decayNum[x - 1]) / (decayNum[x] - decayNum[x - 1]) + 16);
          MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
        }
        else
          MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness());
      }
      else
      {
        if(x == 0 && arpeggiator->decayNow >= 127)
          MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness());
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
          MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness());
        
        if (decayNum[x - 1] < velDecay.byte2 && decayNum[x] >= velDecay.byte2)
        {
          uint8_t scale = (int)((255 - 16) * (velDecay.byte2 - decayNum[x - 1]) / (decayNum[x] - decayNum[x - 1]) + 16);
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
      velDecay.byte2 = decayNum[ui.x];
      return true;
    }
    if(keyInfo->state == HOLD)
    {
      MatrixOS::UIInterface::TextScroll("-" + std::to_string(decayNum[ui.x]), COLOR_RED);
      return true;
    }
    return false;
  }

  void VarSync()
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      velocity[i].SyncTo(&arpeggiator->config->velocity[i]);
      knobPtr[i]->SyncTo(arpeggiator->configPtr[i]);
    }
    arpeggiator->config->syncBeat = syncBeat;
    arpeggiator->config->skip = skip;
    arpeggiator->config->forBackward = forBackward;
    arpeggiator->config->repeatEnds = repeatEnds;

    if(arpeggiator->inputList.empty())
    {
      for(uint8_t i = 0; i < 8; i++)
      {
        if(arpeggiator->configPrv[i] != *(arpeggiator->configPtr[i]))
        {
          arpeggiator->activeLabel = i;
          arpeggiator->configPrv[i] = *(arpeggiator->configPtr[i]);
          MLOGD("Arpeggiator", "Arp Config %d Changed.", arpeggiator->activeLabel);
        }
      }
    }
  }

  void VarGet()
  {
    for (uint8_t i = 0; i < 8; i++)
    {
      velocity[i].Get(&arpeggiator->config->velocity[i]);
      knobPtr[i]->Get(arpeggiator->configPtr[i]);
      arpeggiator->configPrv[i] = *arpeggiator->configPtr[i];
    }
    syncBeat = arpeggiator->config->syncBeat;
    skip = arpeggiator->config->skip;
    forBackward = arpeggiator->config->forBackward;
    repeatEnds = arpeggiator->config->repeatEnds;
    arpeggiator->syncBeatPrev = syncBeat;
    arpeggiator->skipPrev = skip;
    arpeggiator->forBackwardPrev = forBackward;
    arpeggiator->repeatEndsPrev = repeatEnds;
  }

  KnobConfig rate;                        // 1/16, 1/12, 1/8, 1/6, 1/4, 1/3, 1/2, 1, 2, 3, 4, 6, 8, 12, 16, 32
  KnobConfig patternLength;               // 1 - 8
  KnobConfig chance;                      // 5 - 100
  KnobConfig gate;                        // 5% , 10%, 15%, 20%, 30%, 40%, def 50%, 60%, 70%, 80%, 90%, 100%, 150%, 200%, 300%, 400%
  KnobConfig type;                        // 0 - 11
  KnobConfig octaveRange;                 // 1 - 6 
  KnobConfig noteRepeat;                  // 1 - 8
  KnobConfig velDecay;                    // 0 - 48
  KnobConfig velocity[8];
  bool syncBeat;
  bool skip;
  bool forBackward;
  bool repeatEnds;

  void KnobInit()
  {
    for(uint8_t i = 0; i < 8; i++)
      velocity[i] = {.lock = true, .byte2 = 127, .min = 1, .def = 127, . color = COLOR_ORANGE};

    rate              = {.lock = true,  .min = 0,   .max = 15,   .def = 4,   .color = knobColor[0]};
    patternLength     = {.lock = true,  .min = 1,   .max = 8,    .def = 1,   .color = knobColor[1]};
    chance            = {.lock = true,  .min = 5,   .max = 100,  .def = 100, .color = knobColor[2]};
    gate              = {.lock = true,  .min = 0,   .max = 15,   .def = 6,   .color = knobColor[3]};
    type              = {.lock = true,  .min = 0,   .max = 11,   .def = 0,   .color = knobColor[4]};
    octaveRange       = {.lock = true,  .min = 1,   .max = 6,    .def = 1,   .color = knobColor[5]};
    noteRepeat        = {.lock = true,  .min = 1,   .max = 8,    .def = 1,   .color = knobColor[6]};
    velDecay          = {.lock = true,  .min = 0,   .max = 48,   .def = 0,   .color = knobColor[7]};
  }
};