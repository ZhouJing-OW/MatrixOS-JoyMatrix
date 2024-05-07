#pragma once
#include "MidiCenter.h"
#include "SeqData.h"
#include "Sequencer.h"
#include "NodeUI.h"

namespace MatrixOS::MidiCenter
{
  extern SEQ_DataStore*     seqData;

  class SequencerUI : public NodeUI {
    
    enum SeqUIMode : uint8_t {NORMAL, TRIPLET, SETTING};

    //-----------------------------------NORMAL-----------------------------------//

    const Color stepColor[2]          ={Color(LAWN_LS),             Color(LAWN)};                                    // poly, mono
    const Color backColor[3]          ={Color(BLANK),               Color(BLUE).Scale(8),    Color(CYAN).Scale(8)};  // invalid Step, poly track, mono track
    const Color barColor[3]           ={Color(DEEPSKY).Scale(8),    Color(BLUE_LS),          Color(BLUE)};           // noBar, hasBar, hasNote
    const Color playHeadColor[2]      ={Color(GREEN),               Color(ORANGE)};                                  // play, record

          Point     seqPos                = Point(0, 1);                  Dimension seqArea               = Dimension(16, 1);
    const Point     barPos                = Point(6, 0);            const Dimension barArea               = Dimension(BAR_MAX, 1);
    
    const Point     tripletBarPos         = Point(2, 0);          
    const Dimension tripletBarArea        = Dimension(12, 2);

    const Color     reduceBarColor        = Color(RED).Scale(8);
    const Point     reduceBarBtnPos       = Point(5, 0);
    const char      reduceBarName[6]      = "- Bar";
          Color     ReduceBarColor()      { return clip->barMax > 1 ? reduceBarColor : Color(BLANK);}
          BtnFunc   ReduceBarBtn          = [&]()->void {
                                              ResetEditing(true);
                                              if(clip->barMax > 1) clip->barMax -= 1;
                                              if(barNum >= clip->barMax) barNum = clip->barMax - 1;
                                          };

    const Color     autoGrouthColor       = Color(ORANGE);
    const Point     autoGrouthBtnPos      = Point(10, 0);
    const char      autoGrouthName[12]    = "Auto Grouth";
          Color     AutoGrouthColor()     { return Color(autoGrouthColor).ToLowBrightness(transportState.autoGrouth);}
          BtnFunc   AutoGrouthBtn         = [&]()->void {transportState.autoGrouth = !transportState.autoGrouth;};

    // const Color     monoModeColor         = Color(CYAN);
    // const Point     monoModeBtnPos        = Point(1, 0);
    // const char      monoModeName[10]      = "Mono Mode";
    //       Color     MonoModeColor()       { return Color(monoModeColor).ToLowBrightness(monoMode);};
    //       BtnFunc   monoModeBtn           = [&]()->void { ResetEditing(true); monoMode = !monoMode;};

    const Color     tripletColor          = Color(VIOLET);
    const Point     tripletBtnPos         = Point(1, 0);
    const char      tripletName[8]        = "Triplet";
          BtnFunc   TripletBtn            = [&]()->void { ChangeUIMode(TRIPLET);};

    const Color     settingBtnColor       = Color(BLUE_LS);
    const Point     settingBtnPos         = Point(15, 0);
    const char      settingBtnName[8]     = "Setting";
          BtnFunc   settingBtn            = [&]()->void { ChangeUIMode(SETTING);};

    const Color     OffsetColor           = Color(YELLOW);
          Point     OffsetLPos1()         { return Point(0,  editing.point.y > 0 ? editing.point.y - 1 : 1);}
          Point     OffsetLPos2()         { return Point(14, editing.point.y > 0 ? editing.point.y - 1 : 1);}
          BtnFunc   OffsetLBtn            = [&]()->void { seqData->SetOffset(editing.pos, std::max(seqData->GetOffset(editing.pos) - 1 , -5));};

          Point     OffsetRPos1()         { return Point(1,  editing.point.y > 0 ? editing.point.y - 1 : 1);}
          Point     OffsetRPos2()         { return Point(15, editing.point.y > 0 ? editing.point.y - 1 : 1);}
          BtnFunc   OffsetRBtn            = [&]()->void { seqData->SetOffset(editing.pos, std::min(seqData->GetOffset(editing.pos) + 1 , 5)); };

    //-----------------------------------EDITING-----------------------------------//

    const Color settingColor[4]       = {GREEN,      LAWN,       TURQUOISE,  BLUE};
    const Color LabelColor[4]         = {BLUE,       BLUE,       BLUE,       BLUE};
    const char  settingName[4][9]     = {"Speed",    "Gate",     "Quantize", "Step/bar"};
    const Color speedColor[10]        = {GREEN_HL,   GREEN_HL,   GREEN_HL,   GREEN_HL,   GREEN,      GREEN,   GREEN_HL,   GREEN_HL,   GREEN_HL,   GREEN_HL };
    const char  speedName[10][5]      = {"4x",       "3x",       "2x",       "1.5x",     "1x",       "1x",       "2/3",      "1/2",      "1/4",      "1/8"};

    const Point     settingLabelPos       = Point(6, 0);            const Dimension settingLabelArea      = Dimension(4, 1);
    const Point     speedSetPos           = Point(3, 1);            const Dimension speedSetArea          = Dimension(10, 1);
    const Point     gateSetPos            = Point(3, 1);            const Dimension gateSetArea           = Dimension(10, 1);
    const Point     quantizeSetPos        = Point(2, 1);            const Dimension quantizeSetArea       = Dimension(11, 1);
    const Point     stepSetPos            = Point(0, 1);            const Dimension stepSetArea           = Dimension(16, 1);

    const Color     backBtnColor          = Color(BLUE).ToLowBrightness();
    const Point     backBtnPos            = Point(15, 0);
    const char      backBtnName[5]        = "Back";
          BtnFunc   backBtn               = [&]()->void { ChangeUIMode(NORMAL);};

    //-------------------------------------------------------------------------------//      

    struct Editing {
      SEQ_Pos pos           = SEQ_Pos(0);
      Point   point         = Point(0xFFF, 0xFFF);
      uint32_t time         = 0;
      int16_t step          = -1;
      uint8_t note          = 255;
      uint8_t noteCount     = 0;
      int16_t velocity      = -1;
      std::bitset<BAR_MAX * STEP_MAX> gateMap;
    } editing;

    SeqUIMode   mode          = NORMAL;
    Sequencer*  sequencer     = nullptr;
    SEQ_Clip*   clip          = nullptr;
    uint8_t     clipNum       = 0;
    uint8_t     clipNumPrv    = 0;
    int8_t      barNum        = 0;
    int8_t      barCopyFrom   = -1;
    int8_t      scroll        = -1;
    uint8_t     scrollMax     = 1;
    uint8_t     settingLabel  = 0;
    PadType     lastPadType   = PIANO_PAD;
    bool        largeSeq      = false;
    bool        monoMode      = false;

    KnobConfig velocityKnob   = {.lock = true, .data{.varPtr = &editing.velocity}, .min = 1, .max = 127, .def = 127, .color = Color(LAWN)  };

    KnobConfig speedKnob      = {.lock = true, .data{.varPtr = nullptr},           .min = 0, .max = 9,   .def = 4,   .color = settingColor[0] };
    KnobConfig gateKnob       = {.lock = true, .data{.varPtr = nullptr},           .min = 10,.max = 100, .def = 50,  .color = settingColor[1] };
    KnobConfig QuantizeKnob   = {.lock = true, .data{.varPtr = nullptr},           .min = 0, .max = 100, .def = 0,   .color = settingColor[2] };
    KnobConfig stepKnob       = {.lock = true, .data{.varPtr = nullptr},           .min = 8, .max = 16,  .def = 16,  .color = settingColor[3] };

    std::vector<KnobConfig*> settingKnob= {nullptr, nullptr, &speedKnob, &gateKnob, &QuantizeKnob, &stepKnob, nullptr, nullptr};

  public : SequencerUI() {
    channel = MatrixOS::UserVar::global_channel;
    channelPrv = channel;
    if (channelConfig->padType[channel] == DRUM_PAD)
      monoMode = true;
    else
      monoMode = false;
    sequencer = (Sequencer*)GetNodePtr(NODE_SEQ);
  }

    ~SequencerUI() { seqData->Capture_EndEditing(); Device::AnalogInput::DisableUpDown();}

    virtual void SetNull() {}
    virtual bool SetKnobPtr() { knobPtr.clear(); return false; }
    virtual Dimension GetSize() { return dimension; }

    virtual void ChangeUIMode(SeqUIMode mode) 
    {
      switch(mode)
      { 
        case NORMAL: break;
        case TRIPLET: 
          Device::AnalogInput::DisableUpDown();
          break;
        case SETTING: 
          settingLabel = 0;
          Device::AnalogInput::DisableUpDown();
          break;
      }

      this->mode = mode;
      ResetEditing(true);
    }

    virtual bool Render(Point origin) 
    {
      if (!seqData) return false;

      StateCheck(origin);
      
      switch(mode)
      {
        case NORMAL:   
          if(!largeSeq)
          { 
            BarRender(origin, barPos);
            ButtonRender(origin, reduceBarBtnPos,  ReduceBarColor() );
            ButtonRender(origin, autoGrouthBtnPos, AutoGrouthColor());
            ButtonRender(origin, settingBtnPos,    settingBtnColor  );
          } 
          SeqRender(origin, seqPos);       
          break;
        case TRIPLET:  
          break;
        case SETTING:
          LabelRender (settingLabelArea, origin, settingLabelPos, settingName, LabelColor, settingLabel, true);
          ButtonRender(origin, backBtnPos, backBtnColor);
          switch(settingLabel)
          {
            case 0:
              LabelRender(speedSetArea, origin, speedSetPos, speedName, speedColor, clip->speed);
              break;
            case 1:
              ValueBarRender(gateSetArea, origin, gateSetPos, settingColor[1], Color(settingColor[1]).ToLowBrightness(), clip->gate, 10, 100);
              break;
            case 2:
              ValueBarRender(quantizeSetArea, origin, quantizeSetPos, settingColor[2], Color(settingColor[2]).ToLowBrightness(), clip->quantize, 0, 100, Color(RED).Scale(8));
              break;
            case 3:
              SeqRender(origin, seqPos);
              break;
          }
      }
      

      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
    {
      if (!seqData) return false;

      switch(mode)
      {
        case NORMAL:
          if(InArea(xy, seqPos, seqArea))
            return SeqKeyEvent(xy, seqPos, keyInfo);
          if(!largeSeq)
          {
            if(InArea(xy, barPos, barArea))
              return BarKeyEvent(xy, barPos, keyInfo);
            if(InArea(xy, reduceBarBtnPos,  Dimension(1, 1)) && clip->barMax > 1)
              return ButtonKeyEvent(xy, reduceBarBtnPos,  keyInfo, reduceBarColor,  reduceBarName , ReduceBarBtn );
            if(InArea(xy, autoGrouthBtnPos, Dimension(1, 1)))
              return ButtonKeyEvent(xy, autoGrouthBtnPos, keyInfo, autoGrouthColor, autoGrouthName, AutoGrouthBtn);
            if(InArea(xy, settingBtnPos,    Dimension(1, 1)))
              return ButtonKeyEvent(xy, settingBtnPos,    keyInfo, settingBtnColor, settingBtnName, settingBtn   );
          }
          return false;
        case TRIPLET:
          return false;
        case SETTING:
          if(InArea(xy, settingLabelPos, settingLabelArea))
            return LabelKeyEvent(settingLabelArea, xy, settingLabelPos, keyInfo, settingName, settingColor, settingLabel, 0, 3);
          if(InArea(xy, backBtnPos, Dimension(1, 1)))
            return ButtonKeyEvent(xy, backBtnPos, keyInfo, backBtnColor, backBtnName, backBtn);
          switch(settingLabel)
          {
            case 0:
              if(InArea(xy, speedSetPos, speedSetArea))
                return LabelKeyEvent(speedSetArea, xy, speedSetPos, keyInfo, speedName, speedColor, clip->speed, 0, 9);
              break;
            case 1:
              if(InArea(xy, gateSetPos, gateSetArea))
                return ValueBarKeyEvent(gateSetArea, xy, gateSetPos, keyInfo, settingColor[1], clip->gate, 10, 100);
              break;
            case 2:
              if(InArea(xy, quantizeSetPos, quantizeSetArea))
                return ValueBarKeyEvent(quantizeSetArea, xy, quantizeSetPos, keyInfo, settingColor[2], clip->quantize, 0, 100);
              break;
            case 3:
              if(InArea(xy, seqPos, seqArea))
                return SeqKeyEvent(xy, seqPos, keyInfo);
              break;
          }
          return false;
      }
      return false;
    }
  
  private:
    void StateCheck(Point origin)
    {
      channel = MatrixOS::UserVar::global_channel;
      clipNum = seqData->EditingClip(channel);
      clip    = seqData->Clip(channel, clipNum);

      if (channel != channelPrv || clipNumPrv != clipNum)                                  // Check clip change
      {
        if (channel != channelPrv) sequencer = (Sequencer*)GetNodePtr(NODE_SEQ);
        channelPrv = channel;
        clipNumPrv = clipNum;
        ResetEditing(true);
        barNum = 0;
        ChangeUIMode(NORMAL);
      }

      PadType padTypeNow = (PadType)channelConfig->padType[channel];
      if(padTypeNow != lastPadType)                                                        // Check pad type change
      {
        lastPadType = padTypeNow;
        if(padTypeNow == DRUM_PAD) monoMode = true;
        else monoMode = false;
      }
      
      SaveVelocity(editing.note);                                                          // Check velocity edited.
      
      if(mode == NORMAL)                                                                   // Check direct pad
      {
        if(!largeSeq)                                                                        
        {
          uint8_t barMax = clip->barMax;
          Device::AnalogInput::SetUpDown(&barNum, barMax - 1, barMax > 1 ? -1 : 0);
          if (barNum == -1) LargeSeq();
        }
        else
        {
          Device::AnalogInput::SetUpDown(&barNum, 1);
          if (barNum > 0 || clip->barMax < 2) SmallSeq();
        }
      }

      if(editing.point == Point(0xFFF, 0xFFF))  return;    
                  
      if(!Device::KeyPad::GetKey(Device::KeyPad::XY2ID(editing.point - origin))->active()) // Check editing point button release
      {
        MLOGD("SEQ", "Reset Editing");
        ResetEditing(true);
      }

      if(monoMode)                                                                         // Check editing note change in mono mode
      {
        if (editing.note != channelConfig->activeNote[channel])
        {
          Device::AnalogInput::DisableDial();
          SaveVelocity(editing.note);
          editing.note = channelConfig->activeNote[channel];
          editing.velocity = seqData->GetVelocity(editing.pos, editing.note);
          if(!seqData->FindNote(editing.pos, editing.note))
            editing.gateMap.reset();
          else
          {
            GateRenderMap(seqData->GetGate(editing.pos, editing.note));
            EditVelocity(editing.point, editing.pos);
          }
          return;
        }
      }

      if(seqData->NoteCount(editing.pos, editing.note) != editing.noteCount)               // Check NoteCount change
      {
        if(!seqData->NoteCount(editing.pos, editing.note))
          editing.gateMap.reset();
        else
          GateRenderMap(seqData->GetGate(editing.pos, editing.note));
        
        if(seqData->NoteCount(editing.pos, editing.note) > editing.noteCount)                            
          EditVelocity(editing.point, editing.pos);
          
        editing.noteCount = seqData->NoteCount(editing.pos, editing.note);
      }
      if(!editing.noteCount) Device::AnalogInput::DisableDial();      
    }

    virtual void LargeSeq()
    {
      largeSeq = true;
      uint8_t barMax = clip->barMax;
      if(barMax > 2) fullScreen = true;
      dimension = Dimension(16, barMax > 2 ? 4 : 2);
      seqPos = Point(0, 0);
      seqArea.y = barMax > 2 ? 4 : 2;
      ResetUI();
    }

    virtual void SmallSeq()
    {
      largeSeq = false;
      fullScreen = false;
      dimension = Dimension(16, 2);
      seqPos = Point(0, 1);
      seqArea.y = 1;
      ResetUI();
    }

    virtual void ResetUI()
    {
      mode = NORMAL;
      barNum = 0;
      ResetEditing(true);
    }

    void SeqRender(Point origin, Point offset)
    {
      bool recording = transportState.record;
      for(uint8_t x = 0; x < seqArea.x; x++) {
        for(uint8_t y = 0; y < seqArea.y; y++) {
          Point xy = origin + offset + Point(x, y);
          uint8_t stepNum = x + y * dimension.x + barNum * STEP_MAX;
          
          if (stepNum >= clip->barMax * STEP_MAX) 
          {
            MatrixOS::LED::SetColor(xy, backColor[0]);
            continue;
          }
          if (mode == SETTING)
          {
            MatrixOS::LED::SetColor(xy, backColor[stepNum % STEP_MAX < clip->barStepMax]);
            continue;
          }

          bool playHead = false;  
          if(sequencer && transportState.play) playHead = sequencer->playHead == stepNum;
          
          SEQ_Pos pos = SEQ_Pos(channel, clipNum, stepNum / STEP_MAX, stepNum % STEP_MAX);
          if(stepNum % STEP_MAX >= clip->barStepMax)
            MatrixOS::LED::SetColor(xy, backColor[0]);
          else if (editing.step >= 0 && editing.step == stepNum)   // renderEditing
          {
            Color thisColor = Color(WHITE);
            uint8_t velocity = seqData->GetVelocity(pos, editing.note);
            uint8_t scale =  velocity * 239 / 127 + 16;
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          }
          else if(editing.step >= 0 && editing.gateMap.test(stepNum))
          {
            bool hasNote = seqData->FindNote(pos, editing.note);
            Color thisColor = playHead ? playHeadColor[recording] : stepColor[hasNote];
            MatrixOS::LED::SetColor(xy, thisColor.Scale(127));
          }
          else if (monoMode ? seqData->FindNote(pos, channelConfig->activeNote[channel]) : !seqData->NoteEmpty(pos))
          {
            bool poly = monoMode ? false : seqData->NoteCount(pos) > 1;
            Color thisColor = (editing.time > 0 && editing.time + 200 <= MatrixOS::SYS::Millis()) ? backColor[1 + !monoMode] : stepColor[poly];
            thisColor = playHead ? playHeadColor[recording] : thisColor;
            uint8_t velocity = seqData->GetVelocity(pos, channelConfig->activeNote[channel]);
            uint8_t scale =  velocity * 239 / 127 + 16;
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          }
          else
          {
            bool hasNote = seqData->FindNote(pos);
            Color thisColor = playHead ? playHeadColor[recording] : backColor[1 + monoMode - monoMode * hasNote];
            MatrixOS::LED::SetColor(xy, thisColor);            
          }
        }
      }
    }

    bool SeqKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      uint8_t stepNum = ui.x + ui.y * dimension.x + barNum * STEP_MAX;
      SEQ_Pos pos = SEQ_Pos(channel, clipNum, stepNum / STEP_MAX, stepNum % STEP_MAX);
      //MLOGD("Seq", "pos: %d, channel: %d, Clip: %d, Bar: %d, Number: %d", pos.Pos(), pos.ChannelNum(), pos.ClipNum(), pos.BarNum(), pos.Number());
      if(stepNum >= BAR_MAX * STEP_MAX) return false;

      if (mode == SETTING)
      {
        if(keyInfo->state == RELEASED)
        {
          if (stepNum % STEP_MAX >= 8) clip->barStepMax = stepNum % STEP_MAX + 1;
          if (editing.step >= 0) GateRenderMap(seqData->GetGate(editing.pos, editing.note));
          return true;
        }
        return true;
      }

      if(stepNum % STEP_MAX >= clip->barStepMax || stepNum / STEP_MAX >= clip->barMax) return false;

      switch(monoMode)
      {
        case true:

          if(editing.step == stepNum && keyInfo->state == RELEASED)
          {
            ResetEditing();
            if(!keyInfo->hold)
              seqData->Capture_SaveSingle(pos);
            else
              seqData->Capture_SaveHold(pos);
            return true;
          }

          if(keyInfo->state == PRESSED)
          {
            if (editing.step >= 0)
            {
              if(seqData->FindNote(editing.pos, editing.note))
                SetGate(stepNum);
              return true;
            }
            // if(!seqData->NoteEmpty(pos)) EditVelocity(xy, pos);
            seqData->Capture_Editing(pos);
            SetEditing(pos, xy, stepNum);
            return true;
          }
          return false;
      
        case false:

          if(editing.step == stepNum && keyInfo->state == RELEASED)
          {
            ResetEditing();

            if(keyInfo->shortHold)
            {
              seqData->Capture_SaveHold(pos);
              return true;
            }

            seqData->Capture_SaveClick(pos);
            return true;
          }

          if (keyInfo->state == PRESSED)
          {
            if (editing.step >= 0)
            {
              if(seqData->FindNote(editing.pos, editing.note))
                SetGate(stepNum);
              return true;
            }
            seqData->Capture_Editing(pos);
            SetEditing(pos, xy, stepNum);
            // MLOGD("Seq", "Editing x: %d, y: %d", editingPos.x, editingPos.y);
            return true;
          }
          return true;
      }
    }

    void TripletSeqRender(Point origin, Point offset)
    {

    }

    bool TripletSeqKeyEvent(Point origin, Point offset, KeyInfo* keyInfo)
    {
      return false;
    }
    
    void BarRender(Point origin, Point offset)
    {
      for (uint8_t x = 0; x < BAR_MAX; x++)
      {
        bool hasNote = seqData->FindNote(channel, clipNum, x);
        bool hasBar = x < clip->barMax;
        Color thisColor =  barColor[hasBar + hasBar * hasNote];
        if (x == barNum) thisColor = Color(WHITE).Blink_Color(x == barCopyFrom, thisColor);
        MatrixOS::LED::SetColor(origin + offset + Point(x, 0), thisColor);
      }
    }

    bool BarKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      uint8_t thisBar = ui.x;

      if(thisBar >= BAR_MAX) return false;

      if( keyInfo->state == RELEASED)
      {
        if (barCopyFrom >= 0)
        {
          ResetEditing(true);
          if(thisBar == barCopyFrom)
          {
            if(thisBar >= clip->barMax)
              clip->barMax = thisBar + 1;
            barCopyFrom = -1;
            return true;
          }
          if(barCopyFrom < clip->barMax)
          {
            seqData->CopyBar(SEQ_Pos(channel, clipNum, barCopyFrom, 0), SEQ_Pos(channel, clipNum, thisBar, 0));
            if (ui.x >= clip->barMax) clip->barMax = ui.x + 1;
            return true;
          }
        }
        else
        {
          if(thisBar >= clip->barMax)
            clip->barMax = thisBar + 1;
          barNum = thisBar;
          return true;
        }
      }
      if(barCopyFrom < 0 && keyInfo->state == HOLD)
      {
        barCopyFrom = thisBar;
        return true;
      }

      return false;
    }

    void SetGate(uint8_t stepNum)
    {
      uint8_t invaildStep = STEP_MAX - clip->barStepMax;
      if(stepNum < editing.step) stepNum += clip->barMax * STEP_MAX;
      stepNum -= (stepNum / STEP_MAX - 1 ) * invaildStep;
      uint8_t editNum = editing.step - (editing.step / STEP_MAX - 1 ) * invaildStep;
      uint8_t gate = stepNum - editNum;
      uint8_t currentGate = seqData->GetGate(editing.pos, editing.note);
      seqData->SetGate(editing.pos, gate == currentGate ? 0 : gate, editing.note);
      GateRenderMap(seqData->GetGate(editing.pos, editing.note));
    }

    void GateRenderMap(uint8_t gateLength)
    {
      uint8_t inUseStep = clip->barStepMax;
      uint8_t stepAll = clip->barMax * STEP_MAX;
      uint8_t tail = gateLength / inUseStep * STEP_MAX + gateLength % inUseStep;
      uint8_t edit = editing.step;
      editing.gateMap.reset();
      while (tail)
      {
        edit++; tail--;
        if (edit % STEP_MAX >= inUseStep) edit += STEP_MAX - edit % STEP_MAX;
        if (edit >= stepAll) edit = 0;
        editing.gateMap.set(edit);
      }
    }

    void SaveVelocity(uint8_t note)
    {
      if(editing.velocity > 0 && !Device::AnalogInput::GetDialPtr())
      {
        seqData->SetVelocity(editing.pos, editing.velocity, note);
        if(!seqData->NoteEmpty(editing.pos))
          seqData->Capture_UpdateCap(editing.pos);
        editing.velocity = -1;
      }
    }

    void EditVelocity(Point xy, SEQ_Pos position)
    {
      editing.velocity = seqData->GetVelocity(position, editing.note);
      Device::AnalogInput::UseDial(xy, &velocityKnob);
    }

    void SetEditing(SEQ_Pos pos, Point xy, uint8_t stepNum)
    { 
      editing.point = xy;
      editing.pos = pos;
      editing.step = stepNum;
      editing.note = monoMode ? channelConfig->activeNote[channel] : 255;
      editing.noteCount = seqData->NoteCount(pos, editing.note);
      editing.time = MatrixOS::SYS::Millis();
      GateRenderMap(seqData->GetGate(pos, editing.note));
      if(!seqData->NoteEmpty(pos)) EditVelocity(xy, pos);
    }

    void ResetEditing(bool EndEditing = false)
    {
      editing.point = Point(0xFFF,0xFFF);
      editing.step = -1;
      editing.note = monoMode ? channelConfig->activeNote[channel] : 255;
      editing.noteCount = 0;
      editing.time = 0;
      Device::AnalogInput::DisableDial();
      if(EndEditing) seqData->Capture_EndEditing();
    }
  };
}