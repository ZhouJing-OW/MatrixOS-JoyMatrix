#pragma once
#include "MatrixOS.h"
#include "NodeUI.h"

namespace MatrixOS::MidiCenter
{
  class ChorderUI : public NodeUI
  {
    public:
    Chorder* chorder;
    virtual void SetNull() { chorder = nullptr; }

    ChorderUI () {
      channel = MatrixOS::UserVar::global_channel;
      channelPrv = channel;
      CheckNodeChange(chorder, NODE_CHORD);
      // KnobInit();
    }

    virtual bool SetKnobPtr() { knobPtr.clear(); return false; }

    virtual Color GetColor() { return COLOR_GOLD; }
    virtual Dimension GetSize() { return dimension; }
    
    const Point labelPos        = Point(1, 0);
    const Point maxVoicesPos    = Point(4, 1);
    const Point randomButtonPos = Point(9, 0);
    const Point rangePos        = Point(4, 0);
    const Point seventhPos      = Point(1, 1);
    const Point harmonyPos      = Point(13, 1);
    uint8_t labelWidth = 2, maxVoicesWidth = 8, randomButtonWidth = 3, rangeWidth = 5, seventhWidth = 2, harmonyWidth = 3;
    // const Color knobColor[8]    = { COLOR_CYAN,     COLOR_AZURE,    COLOR_LIME,     COLOR_GREEN,    COLOR_YELLOW,   COLOR_GOLD,     COLOR_ORANGE,   COLOR_RED };
    const Color labelColor[2]   = { COLOR_PURPLE,      COLOR_PURPLE  };
    const char labelName[2][12] = { "Setting",         "Performing", };
    const char rangeName[5][12] = { "Bass -2",         "Base -1",      "1 Octave",      "Treble +1",    "Treble +2"    };
    const char buttonName[3][16]= { "Auto Voicing",    "Random drop",  "Random Treble",  };
    //                                 "50%",          "60%",          "70%",          "80%",          "90%",          "100%", 
    //                                 "150%",         "200%",         "300%",         "400%" };
    // const Color gateColor[16]   = { COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,
    //                                 COLOR_GOLD,     COLOR_GOLD,     COLOR_GOLD,     COLOR_GOLD,     COLOR_GOLD,     COLOR_GOLD,
    //                                 COLOR_ORANGE,   COLOR_ORANGE,   COLOR_ORANGE,   COLOR_ORANGE };     
    // const char typeName[12][16] = { "UP",           "Down",         "Converge",     "Diverge",      "PinkUp",       "PinkDown",       
    //                                 "ThumbUp",      "ThumbDown",    "Random",       "Random Other", "Random Once",  "By Order"};
    // const Color typeColor[12]   = { COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   COLOR_YELLOW,   
    //                                 COLOR_YELLOW,   COLOR_YELLOW,   COLOR_GREEN,    COLOR_GREEN,    COLOR_GREEN,    COLOR_ORANGE};
    // const uint8_t decayNum[16]  = { 0, 1, 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32, 40, 48 };

    virtual bool Render(Point origin)
    {
      CheckNodeChange(chorder, NODE_CHORD);

      Color switchColor = COLOR_WHITE;
      MatrixOS::LED::SetColor(origin + Point(15, 0), switchColor.ToLowBrightness(chorder != nullptr));

      LabelRender(origin + labelPos);

      if (chorder == nullptr)
        return true;

      switch(chorder->activeLabel)
      {
        case 0: 
          MaxVoicesRender(origin + maxVoicesPos);
          RandomButtonRender(origin + randomButtonPos);
          RangeRender(origin + rangePos);
          SeventhRender(origin + seventhPos);
          HamonyRender(origin + harmonyPos);
          break;
        case 1:
          break;
      }

      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
    {
      if(xy == Point(15, 0))
      {
        if (keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          if (chorder == nullptr) On();
          else Off();
        }
        if (keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("Chorder ON/OFF", COLOR_ORANGE);
          return true;
        }
      }

      if (chorder == nullptr) 
        return false;

      if(xy == labelPos || xy == labelPos + Point(1, 0))
        return LableKeyEvent(xy, labelPos, keyInfo);
      
      switch(chorder->activeLabel)
      {
        case 0: 
          if(xy.y == maxVoicesPos.y && xy.x >= maxVoicesPos.x && xy.x < maxVoicesPos.x + maxVoicesWidth)
            return MaxVoicesEvent(xy, maxVoicesPos, keyInfo);
          if(xy.y == randomButtonPos.y && xy.x >= randomButtonPos.x && xy.x < randomButtonPos.x + randomButtonWidth)
            return RandomButtonKeyEvent(xy, randomButtonPos, keyInfo);
          if(xy.y == rangePos.y && xy.x >= rangePos.x && xy.x < rangePos.x + rangeWidth)
            return RangeEvent(xy, rangePos, keyInfo);
          if(xy.y == seventhPos.y && xy.x >= seventhPos.x && xy.x < seventhPos.x + seventhWidth)
            return SeventhKeyEvent(xy, seventhPos, keyInfo);
          if(xy.y == harmonyPos.y && xy.x >= harmonyPos.x && xy.x < harmonyPos.x + harmonyWidth)
            return HamonyKeyEvent(xy, harmonyPos, keyInfo);
          break;
        case 1:
          break;
      }

      return true;
    }

    void LabelRender(Point origin)
    {
      Color activeColor = COLOR_WHITE;
      for(uint8_t x = 0; x < labelWidth; x++)
      {
        Point xy = origin + Point(x, 0);
        Color thisColor = labelColor[x];
        MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness(chorder != nullptr));
        if (chorder == nullptr) continue;
        
        if (x == chorder->activeLabel)
          MatrixOS::LED::SetColor(xy, activeColor);
        if ((chorder->activeLabel == 2 && x == 3) || (chorder->activeLabel == 3 && x == 2))
          MatrixOS::LED::SetColor(xy, activeColor.Scale(127));
      }
    }

    bool LableKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        // MatrixOS::KnobCenter::SetPage(4);
        chorder->activeLabel = ui.x;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(labelName[ui.x], labelColor[ui.x]);
        return true;
      }
      return false;
    }

    # define RETURN_MIN 0
    # define RETURN_MAX 1
    
    uint8_t VoiceRange(bool max_min){
      switch(chorder->config->trebleRange - chorder->config->bassRange) 
      {
        case 0: return max_min == RETURN_MIN ? 2 : 3 + chorder->config->seventh;
        case 1: return max_min == RETURN_MIN ? 3 : 5;
        case 2: return max_min == RETURN_MIN ? 4 : 7;
        case 3: 
        case 4: return max_min == RETURN_MIN ? 5 : 8;
        default: return max_min == RETURN_MIN ? 2 : 8;
      }
    }

    void MaxVoicesRender(Point origin)
    {
      Color numColor = chorder->config->seventh ? COLOR_GOLD : COLOR_YELLOW;
      Color voidColor = COLOR_RED;
      for(uint8_t x = 0; x < maxVoicesWidth; x++)
      {
        Point xy = origin + Point(x, 0);
        if ( x + 1 > VoiceRange(RETURN_MAX))
          MatrixOS::LED::SetColor(xy, voidColor.ToLowBrightness());
        else if (x + 1 < VoiceRange(RETURN_MIN))
          MatrixOS::LED::SetColor(xy, COLOR_ORANGE);
        else
          MatrixOS::LED::SetColor(xy, numColor.ToLowBrightness( x + 1 <= chorder->config->maxVoices));
      }
    }

    bool MaxVoicesEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      bool inRange = ui.x + 1 <= VoiceRange(RETURN_MAX) && ui.x + 1 >= VoiceRange(RETURN_MIN);
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        if (inRange) chorder->config->maxVoices = ui.x + 1;
      }
      if(keyInfo->state == HOLD)
      {
        string text = inRange ? std::to_string(ui.x + 1) + "Voices" : "Out of Range";
        MatrixOS::UIInterface::TextScroll(text , COLOR_ORANGE);
        return true;
      }
      return false;
    }

    void RangeRender(Point origin)
    {
      Color bassColor = COLOR_BLUE;
      Color trebleColor = COLOR_AZURE;
      MatrixOS::LED::SetColor(origin + Point(0, 0), bassColor.ToLowBrightness(chorder->config->bassRange == 0));
      MatrixOS::LED::SetColor(origin + Point(1, 0), bassColor.ToLowBrightness(chorder->config->bassRange <= 1));
      MatrixOS::LED::SetColor(origin + Point(2, 0), trebleColor);
      MatrixOS::LED::SetColor(origin + Point(3, 0), trebleColor.ToLowBrightness(chorder->config->trebleRange >= 3));
      MatrixOS::LED::SetColor(origin + Point(4, 0), trebleColor.ToLowBrightness(chorder->config->trebleRange == 4));
    }

    bool RangeEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        switch(ui.x)
        {
          case 0: chorder->config->bassRange = 0; break;
          case 1: chorder->config->bassRange = 1; break;
          case 2: 
            chorder->config->bassRange = 2; 
            chorder->config->trebleRange = 2;
            chorder->config->randomDrop = false;
            chorder->config->randomTreble = false;
            chorder->config->autoVoicing = false;
            break;
          case 3: chorder->config->trebleRange = 3; break;
          case 4: chorder->config->trebleRange = 4; break;
        }
        uint8_t max = VoiceRange(RETURN_MAX);
        uint8_t min = VoiceRange(RETURN_MIN);
        if(chorder->config->maxVoices > max) chorder->config->maxVoices = max;
        else if(chorder->config->maxVoices < min) chorder->config->maxVoices = min;
      }

      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(rangeName[ui.x], ui.x < 2 ? COLOR_BLUE : COLOR_AZURE);
        return true;
      }
      return false;
    }

    void SeventhRender(Point origin)
    {
      Color thridColor = COLOR_YELLOW;
      Color SeventhColor = COLOR_GOLD;
      MatrixOS::LED::SetColor(origin + Point(0, 0), thridColor.ToLowBrightness(chorder->config->seventh == 0));
      MatrixOS::LED::SetColor(origin + Point(1, 0), SeventhColor.ToLowBrightness(chorder->config->seventh == 1));
    }

    bool SeventhKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        chorder->config->seventh = ui.x;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        if (chorder->config->seventh) MatrixOS::UIInterface::TextScroll(ui.x == 0 ? "Third" : "Seventh", COLOR_LIME);
        else MatrixOS::UIInterface::TextScroll("Third", COLOR_GREEN);
        return true;
      }
      return false;
    }

    void HamonyRender(Point origin)
    {
      Color harmonyColor = COLOR_YELLOW;
      MatrixOS::LED::SetColor(origin + Point(0, 0), harmonyColor.ToLowBrightness(chorder->config->harmony == 50));
      MatrixOS::LED::SetColor(origin + Point(1, 0), harmonyColor.ToLowBrightness(chorder->config->harmony == 75));
      MatrixOS::LED::SetColor(origin + Point(2, 0), harmonyColor.ToLowBrightness(chorder->config->harmony == 100));
    }

    bool HamonyKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        chorder->config->harmony = ui.x * 25 + 50;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(std::to_string(ui.x * 25 + 50) + "% Harmony", COLOR_YELLOW);
        return true;
      }
      return false;
    }

    void RandomButtonRender(Point origin)
    {
      Color bassColor = COLOR_BLUE;
      Color trebleColor = COLOR_AZURE;
      Color autoColor = COLOR_GREEN;
      if(chorder->config->randomDrop || chorder->config->randomTreble)
        MatrixOS::LED::SetColor(origin + Point(0, 0), autoColor.ToLowBrightness(chorder->config->autoVoicing));
      MatrixOS::LED::SetColor(origin + Point(1, 0), bassColor.ToLowBrightness(chorder->config->randomDrop));
      MatrixOS::LED::SetColor(origin + Point(2, 0), trebleColor.ToLowBrightness(chorder->config->randomTreble));
      
    }

    bool RandomButtonKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        switch(ui.x)
        {
          case 0: 
            if(chorder->config->randomDrop || chorder->config->randomTreble)
              chorder->config->autoVoicing = !chorder->config->autoVoicing; 
            break;
          case 1: chorder->config->randomDrop = !chorder->config->randomDrop; break;
          case 2: chorder->config->randomTreble = !chorder->config->randomTreble; break;
          
        }
        if(chorder->config->trebleRange - chorder->config->bassRange == 0) 
          chorder->config->bassRange = 1;
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(buttonName[ui.x], COLOR_LIME);
        return true;
      }
      return false;
    }

    // void PatternRender(Point origin)
    // {
    //   Color thisColor = COLOR_ORANGE;
    //   Color backColor = COLOR_BLUE;
    //   Color skipColor = COLOR_RED;
    //   Color lengthColor = skip ? COLOR_RED : knobColor[4]; 
    //   for(uint8_t x = 0; x < 16; x++)
    //   {
    //     Point xy = origin + Point(x, 0);
    //     if (x == 15)
    //       MatrixOS::LED::SetColor(xy, skipColor.ToLowBrightness(skip));
    //     else if (x > 3 && x < 12)
    //     {
    //       uint8_t i = x - 4;
    //       if(i == arpeggiator->currentStep && !arpeggiator->inputList.empty())
    //       MatrixOS::LED::SetColor(xy, COLOR_WHITE);
    //       else if(i < arpeggiator->config->patternLength)
    //       {
    //         bool active = bitRead(arpeggiator->config->pattern, i);
    //         uint8_t scale = arpeggiator->config->velocity[i];
    //         MatrixOS::LED::SetColor(xy, active ? thisColor.Scale(scale, 0, 127, 16) : lengthColor.ToLowBrightness());
    //       }
    //       else
    //         MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness());
    //     }
    //     else if (x == 0)
    //       MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness());
    //     else if (x == 1)
    //       MatrixOS::LED::SetColor(xy, lengthColor.ToLowBrightness());
    //     else
    //       MatrixOS::LED::SetColor(xy, COLOR_BLANK);
    //   }
    // }

    // bool PatternKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    // {
    //   Point ui = xy - offset;
    //   if (ui.x == 0)
    //   {
    //     if(keyInfo->state == RELEASED && keyInfo->hold == false)
    //     {
    //       patternLength.byte2 = patternLength.byte2 - 1 > 1 ? patternLength.byte2 - 1 : 1;
    //       return true;
    //     }
    //     if (keyInfo->state == HOLD)
    //     {
    //       MatrixOS::UIInterface::TextScroll("-Step", COLOR_GREEN);
    //       return true;
    //     }
    //   }
    //   if (ui.x == 1)
    //   {
    //     if(keyInfo->state == RELEASED && keyInfo->hold == false)
    //     {
    //       patternLength.byte2 = patternLength.byte2 + 1 < 9 ? patternLength.byte2 + 1 : 8;
    //       return true;
    //     }
    //     if (keyInfo->state == HOLD)
    //     {
    //       MatrixOS::UIInterface::TextScroll("+Step", COLOR_GREEN);
    //       return true;
    //     }
    //   }
    //   if (ui.x > 3 && ui.x < arpeggiator->config->patternLength + 4)
    //   {
    //     if(keyInfo->state == PRESSED)
    //     {
    //       if (bitRead(arpeggiator->config->pattern, ui.x - 4))
    //         Device::AnalogInput::UseDial(xy, &velocity[ui.x - 4]);
    //       return true;
    //     }

    //     if(ui.x != 4 && keyInfo->state == RELEASED && keyInfo->hold == false)
    //     {
    //       arpeggiator->config->pattern ^= (1 << (ui.x - 4));
    //       return true;
    //     }
    //   }
    //   if (ui.x > 4 && ui.x >= arpeggiator->config->patternLength + 4 && ui.x < 12)
    //   {
    //     if(keyInfo->state == RELEASED && keyInfo->hold == false)
    //     {
    //       patternLength.byte2 = ui.x - 3;
    //       return true;
    //     }
    //     if(keyInfo->state == HOLD)
    //     {
    //       MatrixOS::UIInterface::TextScroll("Length", COLOR_GREEN);
    //       return true;
    //     }
    //   }
    //   if(ui.x == 15)
    //   {
    //     if(keyInfo->state == RELEASED && keyInfo->hold == false)
    //     {
    //       skip = !skip;
    //       return true;
    //     }
    //     if(keyInfo->state == HOLD)
    //     {
    //       MatrixOS::UIInterface::TextScroll("Skip", COLOR_RED);
    //       return true;
    //     }
    //   }
      
    //   return false;
    // }

    // void ChanceRender(Point origin)
    // {
    //   Color thisColor = knobColor[5];
    //   Color backColor = skip ? COLOR_RED : COLOR_BLUE;
    //   Color skipColor = COLOR_RED;
    //   Color blinkColor = knobColor[2];
    //   for(uint8_t x = 0; x < 16; x++)
    //   {
    //     Point xy = origin + Point(x, 0);
    //     if(x == 0)
    //       MatrixOS::LED::SetColor(xy, blinkColor.ToLowBrightness());
    //     else if (x == 1)
    //       MatrixOS::LED::SetColor(xy, blinkColor);
    //     else if(x > 2 && x < 13)
    //     {
    //       uint8_t i = x - 3;
    //       if(i >= chance.byte2 / 10)
    //         MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness().Blink_Color((chance.byte2 - 1) % 10 == i, blinkColor));
    //       else
    //         MatrixOS::LED::SetColor(xy, thisColor.Blink_Color((chance.byte2 - 1) % 10 == i, blinkColor));
    //     }
    //     else if(x == 15)
    //       MatrixOS::LED::SetColor(xy, skipColor.ToLowBrightness(skip));
    //     else
    //       MatrixOS::LED::SetColor(xy, COLOR_BLANK);
    //   }
    // }

    // bool ChanceKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    // {
    //   Point ui = xy - offset;
    //   if (ui.x == 0)
    //   {
    //     if(keyInfo->state == RELEASED && keyInfo->hold == false)
    //     {
    //       chance.byte2 = chance.byte2 - 1 > 5 ? chance.byte2 - 1 : 5;
    //       return true;
    //     }
    //     if(keyInfo->state == HOLD)
    //     {
    //       MatrixOS::UIInterface::TextScroll("-1%", COLOR_ORANGE);
    //       return true;
    //     }
    //   }
    //   if (ui.x == 1)
    //   {
    //     if(keyInfo->state == RELEASED && keyInfo->hold == false)
    //     {
    //       chance.byte2 = chance.byte2 + 1 < 100 ? chance.byte2 + 1 : 100;
    //       return true;
    //     }
    //     if(keyInfo->state == HOLD)
    //     {
    //       MatrixOS::UIInterface::TextScroll("+1%", COLOR_ORANGE);
    //       return true;
    //     }
    //   }
    //   if (ui.x > 2 && xy.x < 13)
    //   {
    //     if(keyInfo->state == RELEASED && keyInfo->hold == false)
    //     {
    //       chance.byte2 = (ui.x - 3) * 10 + (chance.byte2 % 10 == 0 ? 10 : chance.byte2 % 10);
    //       return true;
    //     }
    //     if(keyInfo->state == HOLD)
    //     {
    //       string cs = std::to_string(chance.byte2) + "%";
    //       MatrixOS::UIInterface::TextScroll(cs, COLOR_ORANGE);
    //       return true;
    //     }
    //   }
    //   if(ui.x == 15)
    //   {
    //     if(keyInfo->state == RELEASED && keyInfo->hold == false)
    //     {
    //       skip = !skip;
    //       return true;
    //     }
    //     if(keyInfo->state == HOLD)
    //     {
    //       MatrixOS::UIInterface::TextScroll("Skip", COLOR_RED);
    //       return true;
    //     }
    //   }
    //   return false;
    // }

    // void GateRender(Point origin)
    // {
    //   for(uint8_t x = 0; x < 16; x++)
    //   {
    //     Point xy = origin + Point(x, 0);
    //     Color thisColor = gateColor[x];
    //     if (x == gate.byte2)
    //       MatrixOS::LED::SetColor(xy,knobColor[6]);
    //     else if (x < gate.byte2)
    //       MatrixOS::LED::SetColor(xy, gateColor[gate.byte2]);
    //     else
    //       MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
    //   }
    // }

    // bool GateKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    // {
    //   Point ui = xy - offset;
    //   if(keyInfo->state == RELEASED && keyInfo->hold == false)
    //   {
    //     gate.byte2 = ui.x;
    //     return true;
    //   }

    //   if(keyInfo->state == HOLD)
    //   {
    //     MatrixOS::UIInterface::TextScroll(gateName[ui.x], gateColor[ui.x]);
    //     return true;
    //   }
    //   return false;
    // }

    // void DecayRender(Point origin)
    // {
    //   Color thisColor = knobColor[7];
    //   Color backColor = COLOR_BLUE;
    //   Color velColor = COLOR_LIME;
    //   for(uint8_t x = 0; x < 16; x++)
    //   {
    //     Point xy = origin + Point(x, 0);
    //     if(arpeggiator->inputList.empty())
    //     {
    //       if (x == 0)
    //         MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
    //       else if (decayNum[x] < velDecay.byte2)
    //         MatrixOS::LED::SetColor(xy, thisColor.ToLowBrightness());
    //       else if (decayNum[x - 1] < velDecay.byte2 && decayNum[x] >= velDecay.byte2)
    //       {
    //         uint8_t scale = (int)((255 - 16) * (velDecay.byte2 - decayNum[x - 1]) / (decayNum[x] - decayNum[x - 1]) + 16);
    //         MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
    //       }
    //       else
    //         MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness());
    //     }
    //     else
    //     {
    //       if(x == 0 && arpeggiator->decayNow >= 127)
    //         MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness());
    //       else if ( x * 8 < 127 - arpeggiator->decayNow)
    //       {
    //         MatrixOS::LED::SetColor(xy, velColor);
    //       }
    //       else if ((x - 1) * 8 < 127 - arpeggiator->decayNow && x * 8 >= 127 - arpeggiator->decayNow)
    //       {
    //         uint8_t scale = (int)((255 - 16) * (127 - arpeggiator->decayNow - (x - 1) * 8) / 8 + 16);
    //         MatrixOS::LED::SetColor(xy, velColor.Scale(scale));
    //       }
    //       else
    //         MatrixOS::LED::SetColor(xy, backColor.ToLowBrightness());
          
    //       if (decayNum[x - 1] < velDecay.byte2 && decayNum[x] >= velDecay.byte2)
    //       {
    //         uint8_t scale = (int)((255 - 16) * (velDecay.byte2 - decayNum[x - 1]) / (decayNum[x] - decayNum[x - 1]) + 16);
    //         MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
    //       }
    //     }
    //   }
    // }

    // bool DecayKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    // {
    //   Point ui = xy - offset;
    //   if(keyInfo->state == RELEASED && keyInfo->hold == false)
    //   {
    //     velDecay.byte2 = decayNum[ui.x];
    //     return true;
    //   }
    //   if(keyInfo->state == HOLD)
    //   {
    //     MatrixOS::UIInterface::TextScroll("-" + std::to_string(decayNum[ui.x]), COLOR_RED);
    //     return true;
    //   }
    //   return false;
    // }

    // bool syncBeat;
    // bool skip;
    // bool forBackward;
    // bool repeatEnds;
    // KnobConfig type;                        // 0 - 11
    // KnobConfig rate;                        // 1/16, 1/12, 1/8, 1/6, 1/4, 1/3, 1/2, 1, 2, 3, 4, 6, 8, 12, 16, 32
    // KnobConfig octaveRange;                 // 1 - 6 
    // KnobConfig noteRepeat;                  // 1 - 8
    // KnobConfig patternLength;               // 1 - 8
    // KnobConfig chance;                      // 5 - 100
    // KnobConfig gate;                        // 5% , 10%, 15%, 20%, 30%, 40%, def 50%, 60%, 70%, 80%, 90%, 100%, 150%, 200%, 300%, 400%
    // KnobConfig velDecay;                    // 0 - 48
    // KnobConfig velocity[8];

    void KnobInit()
    {
      // for(uint8_t i = 0; i < 8; i++)
      //   velocity[i] = {.lock = true, .byte2 = 127, .min = 1, .def = 127, . color = COLOR_ORANGE};

      // type              = {.lock = true,  .min = 0,   .max = 11,   .def = 0,   .color = knobColor[0]};
      // rate              = {.lock = true,  .min = 0,   .max = 15,   .def = 4,   .color = knobColor[1]};
      // octaveRange       = {.lock = true,  .min = 1,   .max = 6,    .def = 1,   .color = knobColor[2]};
      // noteRepeat        = {.lock = true,  .min = 1,   .max = 8,    .def = 1,   .color = knobColor[3]};
      // patternLength     = {.lock = true,  .min = 1,   .max = 8,    .def = 1,   .color = knobColor[4]};
      // chance            = {.lock = true,  .min = 5,   .max = 100,  .def = 100, .color = knobColor[5]};
      // gate              = {.lock = true,  .min = 0,   .max = 15,   .def = 6,   .color = knobColor[6]};
      // velDecay          = {.lock = true,  .min = 0,   .max = 48,   .def = 0,   .color = knobColor[7]};
    }
  };
}