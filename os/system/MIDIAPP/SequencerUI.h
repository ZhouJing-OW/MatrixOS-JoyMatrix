#pragma once
#include "MidiCenter.h"
#include "SeqData.h"
#include "Sequencer.h"
#include "NodeUI.h"

namespace MatrixOS::MidiCenter
{
  extern SEQ_DataStore*     seqData;

  class SequencerUI : public NodeUI {
    
    enum SeqUIMode : uint8_t {NORMAL, TRIPLET, SETTING, COMPONENT};
    enum EditState : uint8_t {EDIT_NONE, COPY_BAR, COPY_STEP,};

    struct StepEditing {
      bool edited = false;
      bool editing = false;
      SEQ_Pos pos = SEQ_Pos(0);
      Point point = Point(0xFFF, 0xFFF);
      uint32_t time = 0;
      uint8_t note = 255;
      uint8_t noteCount = 0;
      int16_t velocity = -1;
      int16_t lastVelocity = -1;
      std::bitset<BAR_MAX * STEP_MAX> gateMap;
    } stepEditing;

    struct EditBlock {
        EditState state = EDIT_NONE;
        int8_t barStart = -1;
        int8_t barEnd = -1;
        int8_t stepStart = -1;
        int8_t stepEnd = -1;
        int8_t loopEditBar = -1;
        bool barKeyStates[BAR_MAX] = {false};
        uint8_t barKeyCount = 0;
        bool copyKeyHeld = false;
        bool deleteKeyHeld = false;
        Point deleteBarTarget = Point(-1, -1);
        Point deleteStepTarget = Point(-1, -1);
    } editBlock;

    struct ClipSettingValue 
    {
      int16_t speed = 0;
      int16_t gate = 0;
      int16_t quantize = 0;
      int16_t step = 0;
    } tempSetting;

    //-----------------------------------NORMAL-----------------------------------//

    const Color stepColor[2]          ={Color(LAWN_LS),             Color(LAWN)};                                    // poly, mono
    const Color backColor[3]          ={Color(BLANK),               Color(BLUE).Scale(8),    Color(CYAN).Scale(8)};  // invalid Step, poly track, mono track
    const Color barColor[3]           ={Color(VIOLET).Scale(8),     Color(BLUE_LS),          Color(BLUE)};           // noBar, hasBar, hasNote
    const Color loopBarColor[3]       ={Color(RED).Scale(8),     Color(VIOLET_HL),        Color(VIOLET)};         // noBar, hasBar, hasNote
    const Color playHeadColor[2]      ={Color(GREEN),               Color(ORANGE)};                                  // play, record

          Point seqPos                = Point(0, 1);                Dimension seqArea        = Dimension(16, 1);
    const Point barPos                = Point(0, 0);          const Dimension barArea        = Dimension(BAR_MAX, 1);
    
    // const Point     tripletBarPos         = Point(2, 0);          
    // const Dimension tripletBarArea        = Dimension(12, 2);

    // const Color     tripletColor          = Color(VIOLET);
    // const Point     tripletBtnPos         = Point(1, 0);
    // const char      tripletName[8]        = "Triplet";
    //       BtnFunc   TripletBtn            = [&]()->void { ChangeUIMode(TRIPLET);};

    //-----------------------------------SETTING------------------------------------//

    const Color     settingColor[4]       = {GREEN,      LAWN,       TURQUOISE,  BLUE};
    const Color     setLabelColor[4]      = {VIOLET,     VIOLET,     VIOLET,     VIOLET};
    const char      settingName[4][9]     = {"Speed",    "Gate",     "Quantize", "Step/bar"};
    const Color     speedColor[10]        = {GREEN_HL,   GREEN_HL,   GREEN_HL,   GREEN_HL,   GREEN,      GREEN_HL,   GREEN_HL,   GREEN_HL,   GREEN_HL,   GREEN_HL };
    const char      speedName[10][5]      = {"4x",       "3x",       "2x",       "1.5x",     "1x",       "2/3",      "1/2",      "1/4",      "1/8",      "1/16" };

    const Point     settingLabelPos       = Point(6, 3);            const Dimension settingLabelArea      = Dimension(4, 1);
    const Point     speedSetPos           = Point(3, 2);            const Dimension speedSetArea          = Dimension(10, 1);
    const Point     gateSetPos            = Point(3, 2);            const Dimension gateSetArea           = Dimension(10, 1);
    const Point     quantizeSetPos        = Point(2, 2);            const Dimension quantizeSetArea       = Dimension(11, 1);
    const Point     stepSetPos            = Point(0, 2);            const Dimension stepSetArea           = Dimension(16, 1);

    const Color     backBtnColor          = Color(BLUE).DimIfNot();
    const Point     backBtnPos            = Point(15, 0);
    const char      backBtnName[5]        = "Back";
          BtnFunc   backBtn               = [&]()->void { ChangeUIMode(NORMAL);};

    const Color     settingBtnColor       = Color(VIOLET);
    const char      settingBtnName[8]     = "Setting";
          BtnFunc   settingBtn            = [&]()->void { ChangeUIMode(SETTING);};

    const Point     settingBtnPos         = Point(15, 3);

    //-----------------------------------COMPONENT-----------------------------------//

    const Color     compColor[8]          = {TURQUOISE, CYAN,      DEEPSKY,   MAROON,    PURPLE,    VIOLET,    GOLD,      YELLOW};
    const Color     compLabelColor[8]     = {TURQUOISE, TURQUOISE, TURQUOISE, TURQUOISE, PURPLE,    PURPLE,    GOLD,      YELLOW};
    const char      compName[8][9]        = {"Cycle"  , "Retrig",  "Chance",  "Flam",    "OCTAVE",  "PITCH",   "STURM",   "ARP"};

    //-----------------------------------EDITING-----------------------------------//      

    const Point     copyBtnPos           = Point(13, 2);     // 复制按钮移到第四行
    const Color     copyBtnColor         = Color(CYAN);     // 复制按钮颜色
    const char      copyBtnName[5]       = "Copy";

    const Point     deleteBtnPos         = Point(14, 2);     // 删除按钮移到第四行
    const Color     deleteBtnColor       = Color(RED);      // 删除按钮颜色
    const char      deleteBtnName[7]     = "Delete";

    const Point     leftShiftBtnPos      = Point(0, 3);     // 左移按钮位置
    const Color     leftShiftBtnColor[2] = {Color(GOLD), Color(GOLD_HL)};   // 左移按钮颜色
    const char      leftShiftBtnName[5]  = "Left";

    const Point     rightShiftBtnPos     = Point(2, 3);     // 右移按钮位置
    const Color     rightShiftBtnColor[2]= {Color(GOLD), Color(GOLD_HL)};   // 右移按钮颜色
    const char      rightShiftBtnName[6] = "Right";

    const Point     undoBtnPos           = Point(13, 3);     // 撤销按钮位置
    const Color     undoBtnColor         = Color(ORANGE);   // 撤销按钮颜色
    const char      undoBtnName[5]       = "Undo";

    const Point     redoBtnPos           = Point(14, 3);     // 重做按钮位置
    const Color     redoBtnColor         = Color(ORANGE);   // 重做按钮颜色
    const char      redoBtnName[5]       = "Redo";

    const Point     transUpBtnPos    = Point(1, 2);     // 向上转调按钮位置
    const Color     transUpBtnColor[2]  = {Color(PURPLE), Color(PURPLE_HL)};   // 向上转调按钮颜色
    const char      transUpBtnName[3]= "Up";

    const Point     transDownBtnPos  = Point(1, 3);     // 向下转调按钮位置
    const Color     transDownBtnColor[2]= {Color(PURPLE), Color(PURPLE_HL)};   // 向下转调按钮颜色
    const char      transDownBtnName[5]= "Down";

    //-------------------------------------------------------------------------------// 

    SeqUIMode   mode          = NORMAL;
    Sequencer*  sequencer     = nullptr;
    SEQ_Clip*   clip          = nullptr;
    uint8_t     clipNum       = 0;
    uint8_t     clipNumPrv    = 0;
    int8_t      barCopyFrom   = -1;
    int8_t      scroll        = -1;
    uint8_t     scrollMax     = 1;
    uint8_t     settingLabel  = 0;
    uint8_t     compLabel     = 0; 
    PadType     lastPadType   = PIANO_PAD;
    bool        monoMode      = false;
    bool        transOctaveSuccess = false;
    bool        shiftBarSuccess = false;
    bool        hasNotes = false; 

    KnobConfig velocityKnob   = {.lock = true, .data{.varPtr = &stepEditing.velocity}, .min = 1, .max = 127, .def = 127, .color = Color(LAWN)  };

    KnobConfig speedKnob      = {.lock = true, .data{.varPtr = nullptr},           .min = 0, .max = 9,   .def = 4,   .color = settingColor[0] };
    KnobConfig gateKnob       = {.lock = true, .data{.varPtr = nullptr},           .min = 10,.max = 100, .def = 50,  .color = settingColor[1] };
    KnobConfig QuantizeKnob   = {.lock = true, .data{.varPtr = nullptr},           .min = 0, .max = 100, .def = 0,   .color = settingColor[2] };
    KnobConfig stepKnob       = {.lock = true, .data{.varPtr = nullptr},           .min = 8, .max = 16,  .def = 16,  .color = settingColor[3] };

    std::vector<KnobConfig*> settingKnob= {nullptr, nullptr, &speedKnob, &gateKnob, &QuantizeKnob, &stepKnob, nullptr, nullptr};

  public : SequencerUI() {
    channel = MatrixOS::UserVar::global_channel;
    channelPrv = 255;
    if (channelConfig->padType[channel] == DRUM_PAD)
      monoMode = true;
    else
      monoMode = false;
    enableMiniPad = true;
  }

    ~SequencerUI() { seqData->Pick_EndEditing();}

    virtual void SetNull() {}
    virtual bool SetKnobPtr() { knobPtr.clear(); return false; }
    virtual Dimension GetSize() { return dimension; }

    void ChangeUIMode(SeqUIMode mode) 
    {
      switch(mode)
      { 
        case NORMAL: 
          enableMiniPad = true; 
          break;
        case TRIPLET: 
          enableMiniPad = true;
          break;
        case SETTING: 
          settingLabel = 0;
          enableMiniPad = false;
          break;
        case COMPONENT:
          enableMiniPad = false;
          compLabel = 0;
          break;
      }
      this->mode = mode;
      shiftBarSuccess = false;
      transOctaveSuccess = false;
      ResetStepEditing();
    }

    virtual bool Render(Point origin) 
    {
      if (!seqData) return false;

      StateCheck(origin);
      
      switch(mode)
      {
        case NORMAL:   
            BarRender(origin, barPos);
            SeqRender(origin, seqPos);
            if(fullScreen)
            {
                ButtonRender(origin, copyBtnPos, editBlock.copyKeyHeld ? Color(WHITE) : copyBtnColor);
                ButtonRender(origin, deleteBtnPos, editBlock.deleteKeyHeld ? Color(WHITE) : deleteBtnColor);
                ButtonRender(origin, settingBtnPos, Color(settingBtnColor).Dim());
                ButtonRender(origin, leftShiftBtnPos, Color(leftShiftBtnColor[shiftBarSuccess]).DimIfNot(hasNotes));
                ButtonRender(origin, rightShiftBtnPos, Color(rightShiftBtnColor[shiftBarSuccess]).DimIfNot(hasNotes));
                ButtonRender(origin, undoBtnPos, Color(undoBtnColor).DimIfNot(seqData->CanUndo()));
                ButtonRender(origin, redoBtnPos, Color(redoBtnColor).DimIfNot(seqData->CanRedo()));
                ButtonRender(origin, transUpBtnPos, Color(transUpBtnColor[transOctaveSuccess]).DimIfNot(hasNotes));
                ButtonRender(origin, transDownBtnPos, Color(transDownBtnColor[transOctaveSuccess]).DimIfNot(hasNotes));
            }
            break;
        case TRIPLET:  
          break;
        case SETTING:
          BarRender(origin, barPos);
          SeqRender(origin, seqPos);
          if(fullScreen) 
          {
            ButtonRender(origin, settingBtnPos, settingBtnColor);
            LabelRender(settingLabelArea, origin, settingLabelPos, settingName, setLabelColor, settingLabel, true);
            switch(settingLabel)
            {
              case 0:
                LabelRender(speedSetArea, origin, speedSetPos, speedName, speedColor, clip->speed);
                break;
              case 1:
                ValueBarRender(gateSetArea, origin, gateSetPos, settingColor[1], Color(settingColor[1]).DimIfNot(), clip->tair, 10, 100);
                break;
              case 2:
                ValueBarRender(quantizeSetArea, origin, quantizeSetPos, settingColor[2], Color(settingColor[2]).DimIfNot(), clip->quantize, 0, 100, Color(RED).Scale(8));
                break;
              case 3:
                StepSetRender(origin, stepSetPos);
                break;
            }
          }
          break;
        case COMPONENT:
          break;
      }
      

      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
    {
      if (!seqData) return false;

      switch(mode)
      {
        case NORMAL:
          if(fullScreen)
          {
            if(InArea(xy, copyBtnPos, Dimension(1, 1)))
              return CopyBtnKeyEvent(xy, copyBtnPos, keyInfo);
            if(InArea(xy, deleteBtnPos, Dimension(1, 1)))
              return DeleteBtnKeyEvent(xy, deleteBtnPos, keyInfo);
            if(InArea(xy, settingBtnPos, Dimension(1, 1)))
              return SettingBtnKeyEvent(xy, settingBtnPos, keyInfo);
            if(InArea(xy, undoBtnPos, Dimension(1, 1)))
              return UndoBtnKeyEvent(xy, undoBtnPos, keyInfo);
            if(InArea(xy, redoBtnPos, Dimension(1, 1)))
              return RedoBtnKeyEvent(xy, redoBtnPos, keyInfo);
            if(InArea(xy, transUpBtnPos, Dimension(1, 1)))
              return TransBtnKeyEvent(xy, transUpBtnPos, keyInfo, true);
            if(InArea(xy, transDownBtnPos, Dimension(1, 1)))
              return TransBtnKeyEvent(xy, transDownBtnPos, keyInfo, false);
            
            if(editBlock.copyKeyHeld && editBlock.state == EDIT_NONE)
            {
              if(InArea(xy, seqPos, seqArea) && keyInfo->state == PRESSED)
              {
                editBlock.state = COPY_STEP;
                Point ui = xy - Point(0, 1);
                uint8_t localStep = ui.x + ui.y * dimension.x;
                uint8_t globalStep = localStep + clip->activeBar * STEP_MAX;
                editBlock.stepStart = globalStep;
                editBlock.stepEnd = globalStep;
                return true;
              }
              else if(InArea(xy, barPos, barArea) && keyInfo->state == PRESSED)
              {
                editBlock.state = COPY_BAR;
                Point ui = xy - barPos;
                uint8_t thisBar = ui.x;
                editBlock.barStart = thisBar;
                editBlock.barEnd = thisBar;
                return true;
              }
            }
            if(InArea(xy, leftShiftBtnPos, Dimension(1, 1)))
              return ShiftBtnKeyEvent(xy, leftShiftBtnPos, keyInfo, true);
            if(InArea(xy, rightShiftBtnPos, Dimension(1, 1)))
              return ShiftBtnKeyEvent(xy, rightShiftBtnPos, keyInfo, false);
          }

          if(InArea(xy, seqPos, seqArea))
            return SeqKeyEvent(xy, seqPos, keyInfo);
          if(InArea(xy, barPos, barArea))
            return BarKeyEvent(xy, barPos, keyInfo);
          return false;
          break;
        case TRIPLET:
          return false;
        case SETTING:
          if(InArea(xy, seqPos, seqArea))
            return SeqKeyEvent(xy, seqPos, keyInfo);
          if(InArea(xy, barPos, barArea))
            return BarKeyEvent(xy, barPos, keyInfo);
          if(InArea(xy, settingBtnPos, Dimension(1, 1)))
            return SettingBtnKeyEvent(xy, settingBtnPos, keyInfo);
          if(InArea(xy, settingLabelPos, settingLabelArea))
            return LabelKeyEvent(settingLabelArea, xy, settingLabelPos, keyInfo, settingName, settingColor, settingLabel, 0, 3);
          switch(settingLabel)
          {
            case 0:
              if(InArea(xy, speedSetPos, speedSetArea))
                return LabelKeyEvent(speedSetArea, xy, speedSetPos, keyInfo, speedName, speedColor, clip->speed, 0, 9);
              break;
            case 1:
              if(InArea(xy, gateSetPos, gateSetArea))
                return ValueBarKeyEvent(gateSetArea, xy, gateSetPos, keyInfo, settingColor[1], clip->tair, 10, 100);
              break;
            case 2:
              if(InArea(xy, quantizeSetPos, quantizeSetArea))
                return ValueBarKeyEvent(quantizeSetArea, xy, quantizeSetPos, keyInfo, settingColor[2], clip->quantize, 0, 100);
              break;
            case 3:
              if(InArea(xy, stepSetPos, stepSetArea))
                return StepSetKeyEvent(xy, stepSetPos, keyInfo);
              break;
          }
          return false;
        case COMPONENT:
          return false;
      }
      return false;
    }
  
  private:
    void StateCheck(Point origin)
    {
      if (!seqData) return;

      channel = MatrixOS::UserVar::global_channel;
      clipNum = seqData->EditingClip(channel);
      clip    = seqData->Clip(channel, clipNum);

      if (channel != channelPrv || clipNumPrv != clipNum)                                  // Check clip change
      {
        if (channel != channelPrv) sequencer = (Sequencer*)GetNodePtr(NODE_SEQ);
        channelPrv = channel;
        clipNumPrv = clipNum;
        ResetStepEditing();
        ChangeUIMode(NORMAL);
      }

      if (sequencer->recording) {
        clip->activeBar = sequencer->playHead / STEP_MAX;
      }

      if (stepEditing.editing) {
        SEQ_Step* step = seqData->Step(stepEditing.pos);
        if (step) {
          hasNotes = !step->Empty();
        }
      } else {
        hasNotes = !seqData->ClipEmpty(channel, clipNum);
      }

      PadType padTypeNow = (PadType)channelConfig->padType[channel];
      if(padTypeNow != lastPadType)                                                        // Check pad type change
      {
        lastPadType = padTypeNow;
        if(padTypeNow == DRUM_PAD) monoMode = true;
        else monoMode = false;
      }
      
      SaveVelocity(stepEditing.note);                                                          // Check velocity edited.
    
      if (seqData->Comp_InEditing())                                                     // Check component editing
        ChangeUIMode(COMPONENT);
      else if (mode == COMPONENT)
        ChangeUIMode(NORMAL);

      //--------------------------------------EDITING--------------------------------------//

      if(!stepEditing.editing)  
      {
        return; 
      }
                  
      if(!Device::KeyPad::GetKey(Device::KeyPad::XY2ID(stepEditing.point - origin))->active()) // Check editing point button release
      {
        MLOGD("SEQ", "Reset Editing");
        ResetStepEditing();
      }

      if(monoMode)                                                                         // Check editing note change in mono mode
      {
        if (stepEditing.note != channelConfig->activeNote[channel])
        {
          Device::AnalogInput::DisableDial();
          SaveVelocity(stepEditing.note);
          stepEditing.note = channelConfig->activeNote[channel];
          stepEditing.velocity = seqData->GetVelocity(stepEditing.pos, stepEditing.note);
          stepEditing.lastVelocity = stepEditing.velocity;
          if(!seqData->FindNote(stepEditing.pos, stepEditing.note))
            stepEditing.gateMap.reset();
          else
          {
            GateRenderMap(seqData->GetGate(stepEditing.pos, stepEditing.note));
            EditVelocity(stepEditing.point, stepEditing.pos);
          }
          return;
        }
      }

      if(seqData->NoteCount(stepEditing.pos, stepEditing.note) != stepEditing.noteCount)               // Check NoteCount change
      {
        if(!seqData->NoteCount(stepEditing.pos, stepEditing.note))
          stepEditing.gateMap.reset();
        else
          GateRenderMap(seqData->GetGate(stepEditing.pos, stepEditing.note));
        
        if(seqData->NoteCount(stepEditing.pos, stepEditing.note) > stepEditing.noteCount)                            
          EditVelocity(stepEditing.point, stepEditing.pos);
          
        stepEditing.noteCount = seqData->NoteCount(stepEditing.pos, stepEditing.note);
      }
      if(!stepEditing.noteCount) Device::AnalogInput::DisableDial();      
    }

    void ResetUI()
    {
      mode = NORMAL;
      shiftBarSuccess = false;
      transOctaveSuccess = false;
      ResetStepEditing();
    }

    void SeqRender(Point origin, Point offset)
    {
      bool recording = transportState.record;
      for(uint8_t x = 0; x < seqArea.x; x++) {
        for(uint8_t y = 0; y < seqArea.y; y++) {
          Point xy = origin + offset + Point(x, y);
          uint16_t localStep = x + y * dimension.x;
          uint16_t bar  = localStep / STEP_MAX + clip->activeBar;
          uint16_t step = localStep % STEP_MAX;
          uint16_t globalStep = bar * STEP_MAX + step;
          SEQ_Pos pos = SEQ_Pos(channel, clipNum, bar, step);
          
          if (bar >= clip->barMax) 
          {
            MatrixOS::LED::SetColor(xy, backColor[0]);
            continue;
          }
          bool playHead = false;  
          if(sequencer && transportState.play) playHead = sequencer->playHead == globalStep;
          
          if(step % STEP_MAX >= clip->barStepMax)
            MatrixOS::LED::SetColor(xy, backColor[0]);
          else if (stepEditing.editing && stepEditing.pos == pos)   // renderEditing
          {
            Color thisColor = Color(WHITE);
            uint8_t velocity = seqData->GetVelocity(pos, stepEditing.note);
            uint8_t scale =  velocity * 239 / 127 + 16;
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          }
          else if(stepEditing.editing && stepEditing.gateMap.test(globalStep))
          {
            bool hasNote = seqData->FindNote(pos, stepEditing.note);
            Color thisColor = playHead ? playHeadColor[recording] : stepColor[hasNote];
            MatrixOS::LED::SetColor(xy, thisColor.Scale(127));
          }
          else if (monoMode ? seqData->FindNote(pos, channelConfig->activeNote[channel]) : !seqData->NoteEmpty(pos))
          {
            bool poly = monoMode ? false : seqData->NoteCount(pos) > 1;
            Color thisColor = (stepEditing.time > 0 && stepEditing.time + 200 <= MatrixOS::SYS::Millis()) ? backColor[1 + !monoMode] : stepColor[poly];
            thisColor = playHead ? playHeadColor[recording] : thisColor;
            uint8_t velocity = seqData->GetVelocity(pos, channelConfig->activeNote[channel]);
            uint8_t scale =  velocity * 239 / 127 + 16;
            
            // 删除状态下的显示逻辑
            if (editBlock.deleteKeyHeld) {
                Point currentPos = Point(x, y);
                if (currentPos == editBlock.deleteStepTarget) {
                    thisColor = Color(RED);
                    scale = 255;  // 确保红色显示最亮
                }
            }
            
            if (editBlock.state == COPY_STEP) {
                if (editBlock.stepStart >= 0) {
                    int16_t start = std::min(editBlock.stepStart, editBlock.stepEnd);
                    int16_t end = std::max(editBlock.stepStart, editBlock.stepEnd);
                    if (globalStep >= start && globalStep <= end) {
                        thisColor = thisColor.Blink_Color(true, Color(CYAN));
                    }
                }
            }
            
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          }
          else
          {
            bool hasNote = seqData->FindNote(pos);
            Color thisColor = playHead ? playHeadColor[recording] : backColor[1 + monoMode - monoMode * hasNote];

            if (editBlock.state == COPY_STEP) {
                if (editBlock.stepStart >= 0) {
                    int16_t start = std::min(editBlock.stepStart, editBlock.stepEnd);
                    int16_t end = std::max(editBlock.stepStart, editBlock.stepEnd);
                    if (globalStep >= start && globalStep <= end) {
                        thisColor = thisColor.Blink_Color(true, Color(CYAN).Scale(64));
                    }
                }
            }
            MatrixOS::LED::SetColor(xy, thisColor);
          }
        }
      }
    }

    bool SeqKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
        Point ui = xy - offset;
        uint8_t localStep = ui.x + ui.y * dimension.x;
        uint8_t step = localStep % STEP_MAX;
        uint8_t bar  = localStep / STEP_MAX + clip->activeBar;
        uint8_t globalStep = bar * STEP_MAX + step;
        SEQ_Pos pos = SEQ_Pos(channel, clipNum, bar, step);

        // 检查步进是否有效
        if(step >= clip->barStepMax || bar >= clip->barMax) return false;

        // 删除操作
        if (editBlock.deleteKeyHeld) {
            Point ui = xy - offset;
            editBlock.deleteStepTarget = ui;  // 使用 deleteStepTarget
            
            if (keyInfo->state == RELEASED && !keyInfo->hold) {

                if (monoMode) {
                    uint8_t activeNote = channelConfig->activeNote[channel];
                    if (seqData->FindNote(pos, activeNote)) {
                        seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                        seqData->EnableTempSnapshot();
                        seqData->DeleteNote(pos, activeNote);
                    }
                } else {
                    if(seqData->FindNote(pos)) {
                        seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                        seqData->EnableTempSnapshot();
                        seqData->ClearNote(pos);
                    }
                }
                editBlock.deleteStepTarget = Point(-1, -1);  // 重置删除目标位置
                return true;
            }
            return false; 
        }

        // 复制操作
        if (editBlock.copyKeyHeld && editBlock.state == COPY_STEP) {

            if (keyInfo->state == PRESSED) {
                int16_t start = std::min<int16_t>(editBlock.stepStart, editBlock.stepEnd);
                int16_t end = std::max<int16_t>(editBlock.stepStart, editBlock.stepEnd);
                int16_t range = end - start + 1;
                int16_t dstStep = globalStep;
                
                // 计算最大可复制步数，确保类型一致
                int16_t maxSteps = static_cast<int16_t>(BAR_MAX * STEP_MAX - dstStep);
                int16_t copyRange = std::min<int16_t>(range, maxSteps);
                
                if (copyRange > 0) {
                    seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                    seqData->EnableTempSnapshot();
                    for (int16_t i = 0; i < copyRange; i++) {
                        SEQ_Pos srcPos = SEQ_Pos(channel, clipNum, 
                            (start + i) / STEP_MAX,    // 源bar
                            (start + i) % STEP_MAX);   // 源step
                        
                        SEQ_Pos dstPos = SEQ_Pos(channel, clipNum, 
                            (dstStep + i) / STEP_MAX,  // 目标bar
                            (dstStep + i) % STEP_MAX); // 目标step

                        // 确保目标step在有效范围内
                        if ((dstStep + i) % STEP_MAX >= clip->barStepMax) {
                            // 如果超出当前bar，移动到下一个bar的开始
                            dstStep = ((dstStep + i) / STEP_MAX + 1) * STEP_MAX;
                            i--; // 重试当前step
                            continue;
                        }
                        
                        if (monoMode) {
                            uint8_t activeNote = channelConfig->activeNote[channel];
                            if (seqData->FindNote(srcPos, activeNote)) {
                                seqData->CopyNote(srcPos, dstPos, activeNote);
                            }
                        } else {
                            seqData->CopyStep(srcPos, dstPos);
                        }
                    }

                    // 更新最大bar数
                    uint8_t lastBar = (dstStep + copyRange - 1) / STEP_MAX + 1;
                    if (lastBar > clip->barMax) {
                        clip->barMax = lastBar;
                    }
                }
                return true;
            }
            else if (keyInfo->state == HOLD) {
                editBlock.stepEnd = globalStep;
                return true;
            }
            return false;
        }

        // 普通音符编辑
        switch(monoMode)
        {
            case true:
                if(stepEditing.pos == pos && keyInfo->state == RELEASED)
                {
                    if(!keyInfo->hold)
                    {
                        if(seqData->Pick_SaveSingle(pos)) 
                            stepEditing.edited = true;
                        ResetStepEditing();
                        return true;
                    }

                    if(seqData->Pick_SaveHold(pos)) 
                        stepEditing.edited = true;
                    ResetStepEditing();
                    return true;
                }

                if(keyInfo->state == PRESSED)
                {
                    if (stepEditing.editing)
                    {
                        if(seqData->FindNote(stepEditing.pos, stepEditing.note))
                            SetGate(pos);
                        return true;
                    }

                    seqData->Pick_Editing(pos);
                    SetStepEditing(pos, xy);
                    return true;
                }
                return false;

            case false:
                if(stepEditing.pos == pos && keyInfo->state == RELEASED)
                {
                    if(!keyInfo->hold)
                    {
                        if(seqData->Pick_SaveClick(pos)) 
                            stepEditing.edited = true;
                        ResetStepEditing();
                        return true;
                    }

                    if(seqData->Pick_SaveHold(pos)) 
                        stepEditing.edited = true;
                    ResetStepEditing();
                    return true;
                }

                if (keyInfo->state == PRESSED)
                {
                    if (stepEditing.editing)
                    {
                        if(seqData->FindNote(stepEditing.pos, stepEditing.note))
                            SetGate(pos);
                        return true;
                    }
                    seqData->Pick_Editing(pos);
                    SetStepEditing(pos, xy);
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
            bool hasNote = seqData->FindNoteInBar(channel, clipNum, x);
            bool hasBar = x < clip->barMax;
            bool playing = sequencer && transportState.play && sequencer->playHead / STEP_MAX == x;
            
            Color thisColor;
            
            // 在复制或删除状态下显示全范围
            if (editBlock.copyKeyHeld || editBlock.deleteKeyHeld) {
                thisColor = x == clip->activeBar ? Color(WHITE) : barColor[hasBar + hasBar * hasNote];
            }
            // 正常状态下根据 loop 显示
            else if (clip->HasLoop()) {
                if (x >= clip->loopStart && x <= clip->loopEnd) {
                    thisColor = x == clip->activeBar ? Color(WHITE) : loopBarColor[hasBar + hasBar * hasNote];
                } else {
                    if (x < clip->barMax) {
                        thisColor = Color(barColor[1]).Scale(8);
                    } else {
                        thisColor = loopBarColor[0];
                    }
                }
            } else {
                thisColor = x == clip->activeBar ? Color(WHITE) : barColor[hasBar + hasBar * hasNote];
            }
            
            // 删除状态下的显示逻辑
            if (editBlock.deleteKeyHeld && x == editBlock.deleteBarTarget.x && x < clip->barMax) {
                thisColor = Color(RED);
            }
            // 复制状态下的显示逻辑
            else if (editBlock.state == COPY_BAR) {
                if (editBlock.barStart >= 0) {
                    int8_t start = std::min<int8_t>(editBlock.barStart, editBlock.barEnd);
                    int8_t end = std::max<int8_t>(editBlock.barStart, editBlock.barEnd);
                    if (x >= start && x <= end) {
                        thisColor = thisColor.Blink_Color(true, Color(CYAN));
                    }
                }
            }
            
            Color playingColor = playing ? playHeadColor[transportState.record] : thisColor;
            MatrixOS::LED::SetColor(origin + offset + Point(x, 0), playing ? playingColor : thisColor);
        }
    }

    bool BarKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
        Point ui = xy - offset;
        uint8_t thisBar = ui.x;

        if(thisBar >= BAR_MAX) return false;

        // 删除操作
        if (editBlock.deleteKeyHeld) {
            editBlock.deleteBarTarget = Point(ui.x, -1);
            
            if (keyInfo->state == RELEASED && !keyInfo->hold) {
                if (thisBar < clip->barMax) {
                    seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                    seqData->EnableTempSnapshot();
                    seqData->ClearBar(SEQ_Pos(channel, clipNum, thisBar, 0), 
                                    monoMode ? channelConfig->activeNote[channel] : 255);
                    editBlock.deleteBarTarget = Point(-1, -1);
                }
            }
            return true;
        }

        // 复制操作
        if (editBlock.copyKeyHeld && editBlock.state == COPY_BAR) {
            if (keyInfo->state == PRESSED) {
                int8_t start = std::min<int8_t>(editBlock.barStart, editBlock.barEnd);
                int8_t end = std::max<int8_t>(editBlock.barStart, editBlock.barEnd);
                int8_t range = end - start + 1;
                
                int8_t maxRange = static_cast<int8_t>(BAR_MAX - thisBar);
                int8_t copyRange = std::min<int8_t>(range, maxRange);
                
                if (copyRange > 0) {
                    seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                    seqData->EnableTempSnapshot();
                    for (int8_t i = 0; i < copyRange; i++) {
                        seqData->CopyBar(
                            SEQ_Pos(channel, clipNum, start + i, 0),
                            SEQ_Pos(channel, clipNum, thisBar + i, 0)
                        );
                    }
                    if (thisBar + copyRange > clip->barMax) {
                        clip->barMax = thisBar + copyRange;
                    }
                }
                return true;
            }
            else if (keyInfo->state == HOLD) {
                editBlock.barEnd = thisBar;
                return true;
            }
            return false;
        }

        // 更新 bar 按键状态
        if (keyInfo->state == PRESSED) {
            if (!editBlock.barKeyStates[thisBar]) {
                editBlock.barKeyStates[thisBar] = true;
                editBlock.barKeyCount++;
                if (editBlock.barKeyCount == 1) {
                    editBlock.loopEditBar = thisBar;
                }
            }
        }
        else if (keyInfo->state == HOLD) {
            if (thisBar == editBlock.loopEditBar) {
                if (editBlock.barKeyCount > 1) {
                    // 找到最小和最大的按下的 bar
                    int8_t start = BAR_MAX, end = -1;
                    for (int8_t i = 0; i < BAR_MAX; i++) {
                        if (editBlock.barKeyStates[i]) {
                            start = std::min<int8_t>(start, i);
                            end = std::max<int8_t>(end, i);
                        }
                    }
                    
                    // 设置 loop 范围
                    if (end - start + 1 != BAR_MAX) {
                        seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                        seqData->EnableTempSnapshot();
                        clip->SetLoop(start, end);
                    }
                }
                else if (editBlock.barKeyCount == 1) {
                    seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                    seqData->EnableTempSnapshot();
                    if (clip->HasLoop()) {
                        clip->ClearLoop();
                    } else {
                        clip->SetLoop(thisBar, thisBar);
                    }
                }
            }
        }
        else if (keyInfo->state == RELEASED) {
            if (editBlock.barKeyStates[thisBar]) {
                editBlock.barKeyStates[thisBar] = false;
                editBlock.barKeyCount--;
                if (editBlock.barKeyCount == 0) {
                    editBlock.loopEditBar = -1;
                }
            }
            
            // 普通 bar 选择
            if (!keyInfo->hold && editBlock.barKeyCount == 0) {
                if (editBlock.copyKeyHeld || editBlock.deleteKeyHeld || 
                    !clip->HasLoop() || (thisBar >= clip->loopStart && thisBar <= clip->loopEnd)) {
                    if(thisBar >= clip->barMax) {
                        seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                        seqData->EnableTempSnapshot();
                        clip->barMax = thisBar + 1;
                    }
                    clip->activeBar = thisBar;
                }
            }
        }
        return true;
    }

    void StepSetRender(Point origin, Point offset)
    {
      ;
        // 显示当前每个 bar 的 step 数量
        for(uint8_t x = 0; x < 16; x++) {
            Point xy = origin + offset + Point(x, 0);
            if (x < 2) {
              Color thisColor = stepColor[0];
              // 前两个 step 不可选，显示暗色
              MatrixOS::LED::SetColor(xy, thisColor.Dim());
            }
            else if (x < clip->barStepMax) {
                // 当前设置的 step 范围内，显示亮色
                MatrixOS::LED::SetColor(xy, stepColor[0]);
            }
            else {
                // 超出范围的 step，显示最暗色
                MatrixOS::LED::SetColor(xy, backColor[1]);
            }
        }
    }

    bool StepSetKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
        Point ui = xy - offset;
        uint8_t step = ui.x + 1;  // 从1开始计数

        switch(keyInfo->state)
        {
            case PRESSED:
                if (step >= 3 && clip->barStepMax != step) {  // 最小允许3个step
                    clip->barStepMax = step;
                    if (stepEditing.editing) {
                        GateRenderMap(seqData->GetGate(stepEditing.pos, stepEditing.note));
                    }
                }
                return true;

            case HOLD:
                if (step >= 3) {
                    MatrixOS::UIInterface::TextScroll(
                        std::to_string(step) + "/Bar", 
                        settingColor[3]
                    );
                } else {
                    MatrixOS::UIInterface::TextScroll(
                        "Min 3/Bar", 
                        settingColor[3]
                    );
                }
                return true;

            default:
                return false;
        }
    }

    bool DeleteBtnKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
        if (keyInfo->state == PRESSED) {
            editBlock.deleteKeyHeld = true;
            editBlock.state = EDIT_NONE;  // 重置状态
            editBlock.copyKeyHeld = false; // 确保复制状态被清除
            return true;
        }
        else if (keyInfo->state == RELEASED) {
            ResetEditBlock();
            return true;
        }
        return false;
    }

    bool CopyBtnKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
        if (keyInfo->state == PRESSED) {
            editBlock.copyKeyHeld = true;
            editBlock.state = EDIT_NONE;
            return true;
        }
        else if (keyInfo->state == RELEASED) {
            ResetEditBlock();
            return true;
        }
        return false;
    }
    
    bool UndoBtnKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      if(keyInfo->state == PRESSED)
      {
        if(seqData->CanUndo()) seqData->Undo();
        return true;
      }
      return false;
    }

    bool RedoBtnKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      if(keyInfo->state == PRESSED)
      {
        if(seqData->CanRedo()) seqData->Redo();
        return true;
      }
      return false;
    }

    bool SettingBtnKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
        switch(keyInfo->state)
        {
            case PRESSED:
                if (mode == NORMAL) {
                    seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                    tempSetting.speed = clip->speed; tempSetting.gate = clip->tair;
                    tempSetting.quantize = clip->quantize; tempSetting.step = clip->barStepMax;
                    ChangeUIMode(SETTING);
                } else if (mode == SETTING) {
                    if(tempSetting.speed != clip->speed || tempSetting.gate != clip->tair || 
                       tempSetting.quantize != clip->quantize || tempSetting.step != clip->barStepMax)
                    {
                      seqData->EnableTempSnapshot();
                      tempSetting = ClipSettingValue();
                    }
                      ChangeUIMode(NORMAL);
                }
                return true;
            case HOLD:
                MatrixOS::UIInterface::TextScroll("Setting", settingBtnColor);
                return true;
            default:
                return false;
        }
    }

    bool ShiftBtnKeyEvent(Point xy, Point offset, KeyInfo* keyInfo, bool isLeft)
    {
        if (!clip || !hasNotes) {
          if(shiftBarSuccess) shiftBarSuccess = false;
          return false;
        }

        if(keyInfo->state == RELEASED)
        { 
            shiftBarSuccess = false;  // 松开按键时重置状态
            if(!keyInfo->hold)
            { // 短按：移动一步
                if (stepEditing.editing) {
                    stepEditing.edited = true;
                    seqData->ShiftStep(stepEditing.pos, 
                        isLeft ? -1 : 1, 
                        monoMode ? channelConfig->activeNote[channel] : 255);
                } else {
                    seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                    seqData->EnableTempSnapshot();
                    seqData->ShiftClip(SEQ_Pos(channel, clipNum, 0, 0), 
                        isLeft ? -1 : 1, 
                        monoMode ? channelConfig->activeNote[channel] : 255);
                }
                return true;
            }
        }
        if(keyInfo->state == HOLD)
        { // 长按：移动一个 bar
            if (stepEditing.editing) {
                // 如果正在编辑某个step，移动该step一个bar的距离
                stepEditing.edited = true;
                seqData->ShiftStep(stepEditing.pos,
                    isLeft ? -clip->barStepMax : clip->barStepMax,
                    monoMode ? channelConfig->activeNote[channel] : 255);
            } else {
                // 否则移动整个clip
                seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                seqData->EnableTempSnapshot();
                seqData->ShiftClip(SEQ_Pos(channel, clipNum, 0, 0), 
                    isLeft ? -clip->barStepMax : clip->barStepMax,
                    monoMode ? channelConfig->activeNote[channel] : 255);
            }
            shiftBarSuccess = true;  // 设置移动成功状态
            return true;
        }
        return false;
    }

    bool TransBtnKeyEvent(Point xy, Point offset, KeyInfo* keyInfo, bool isUp)
    {
        if (!clip || !hasNotes) {
          if(transOctaveSuccess) transOctaveSuccess = false;
          return false;
        }

        uint16_t scale = 0xFFF;
        uint8_t rootKey = 0;
        int8_t padType = channelConfig->padType[channel];
        if(padType != DRUM_PAD)
        {
          scale = notePadConfig[channelConfig->activePadConfig[channel][padType]].scale;
          rootKey = notePadConfig[channelConfig->activePadConfig[channel][padType]].rootKey;
        }

        if(keyInfo->state == RELEASED)
        { 
            transOctaveSuccess = false;  // 松开按键时重置状态

            if(!keyInfo->hold)
            { // 短按：转调一个音程
                if (stepEditing.editing) {
                    stepEditing.edited = true;
                    seqData->TransposeStep(stepEditing.pos,
                        isUp ? 1 : -1, scale, rootKey,
                        monoMode ? channelConfig->activeNote[channel] : 255);
                } else {
                    seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                    seqData->EnableTempSnapshot();
                    seqData->TransposeClip(SEQ_Pos(channel, clipNum, 0, 0), 
                        isUp ? 1 : -1, scale, rootKey,
                        monoMode ? channelConfig->activeNote[channel] : 255);
                }
            }
            return true;
        }
        if(keyInfo->state == HOLD)
        { // 长按：转调一个八度
            if (stepEditing.editing) {
                // 如果正在编辑某个step，只转调该step一个八度
                stepEditing.edited = true;
                seqData->TransposeStep(stepEditing.pos,
                    isUp ? 12 : -12, scale, rootKey,
                    monoMode ? channelConfig->activeNote[channel] : 255);
            } else {
                // 否则转调整个clip
                seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
                seqData->EnableTempSnapshot();
                seqData->TransposeClip(SEQ_Pos(channel, clipNum, 0, 0), 
                    isUp ? 12 : -12, scale, rootKey,
                    monoMode ? channelConfig->activeNote[channel] : 255);
            }
            transOctaveSuccess = true;  // 设置转调成功状态
            return true;
        }
        return false;
    }

    void SetGate(SEQ_Pos targetPos)
    {
        // 计算全局step位置
        uint16_t editStep = stepEditing.pos.BarNum() * STEP_MAX + stepEditing.pos.Number();
        uint16_t targetStep = targetPos.BarNum() * STEP_MAX + targetPos.Number();
        
        // 确定循环范围
        uint16_t loopStart = (clip->HasLoop() ? clip->loopStart : 0) * STEP_MAX;
        uint16_t loopEnd = ((clip->HasLoop() ? clip->loopEnd : clip->barMax - 1) + 1) * STEP_MAX - 1;
        
        // 计算有效step数
        uint16_t validSteps = 0;
        
        if(targetStep < editStep) {
            // 如果目标在编辑点之前，计算从编辑点到循环结束的有效step
            for(uint16_t step = editStep + 1; step <= loopEnd; step++) {
                if(step % STEP_MAX < clip->barStepMax) {
                    validSteps++;
                }
            }
            
            // 加上从循环起始到目标点的有效step
            for(uint16_t step = loopStart; step <= targetStep; step++) {
                if(step % STEP_MAX < clip->barStepMax) {
                    validSteps++;
                }
            }
        } else {
            // 如果目标在编辑点之后，直接计算中间的有效step
            for(uint16_t step = editStep + 1; step <= targetStep; step++) {
                if(step % STEP_MAX < clip->barStepMax) {
                    validSteps++;
                }
            }
        }
        
        // 获取当前gate值并比较
        uint8_t currentGate = seqData->GetGate(stepEditing.pos, stepEditing.note);
        uint8_t newGate = validSteps;
        
        if(newGate == currentGate) {
            newGate = 0;
        }
        
        seqData->SetGate(stepEditing.pos, newGate, stepEditing.note);
        stepEditing.edited = true;
        GateRenderMap(newGate);
    }

    void GateRenderMap(uint8_t gateLength)
    {
        stepEditing.gateMap.reset();
        if(gateLength == 0) return;
        
        // 设置起始音符位置
        uint16_t startStep = stepEditing.pos.BarNum() * STEP_MAX + stepEditing.pos.Number();
        stepEditing.gateMap.set(startStep);
        
        // 确定循环范围（以step为单位）
        uint16_t loopStart = (clip->HasLoop() ? clip->loopStart : 0) * STEP_MAX;
        uint16_t loopEnd = ((clip->HasLoop() ? clip->loopEnd : clip->barMax - 1) + 1) * STEP_MAX - 1;
        
        // 从起始音符后一个step开始计算
        uint16_t remainLength = gateLength;
        uint16_t currentStep = startStep + 1;
        
        while(remainLength > 0) {
            // 如果超出循环范围，回到循环起始位置
            if(currentStep > loopEnd) {
                currentStep = loopStart;
            }
            
            // 如果当前step在有效范围内（小于当前bar的barStepMax）
            if(currentStep % STEP_MAX < clip->barStepMax) {
                stepEditing.gateMap.set(currentStep);
                remainLength--;
            }
            
            currentStep++;
            
            // 防止无限循环（如果所有可用step都已经设置）
            if(currentStep == startStep) break;
        }
    }

    void SaveVelocity(uint8_t note)
    {
      if(stepEditing.velocity != stepEditing.lastVelocity)
      {
        stepEditing.edited = true;
        seqData->SetVelocity(stepEditing.pos, stepEditing.velocity, note);
        if(!seqData->NoteEmpty(stepEditing.pos))
          seqData->Pick_Update(stepEditing.pos);
        stepEditing.lastVelocity = stepEditing.velocity;
      }
    }

    void EditVelocity(Point xy, SEQ_Pos position)
    {
      stepEditing.velocity = seqData->GetVelocity(position, stepEditing.note);
      stepEditing.lastVelocity = stepEditing.velocity;
      Device::AnalogInput::UseDial(xy, &velocityKnob);
    }

    void SetStepEditing(SEQ_Pos pos, Point xy)
    { 
        seqData->CreateTempSnapshot(SEQ_Pos(channel, clipNum, 0, 0));
        stepEditing.editing = true;
        stepEditing.point = xy;
        stepEditing.pos = pos;
        stepEditing.note = monoMode ? channelConfig->activeNote[channel] : 255;
        stepEditing.noteCount = seqData->NoteCount(pos, stepEditing.note);
        stepEditing.time = MatrixOS::SYS::Millis();
        GateRenderMap(seqData->GetGate(pos, stepEditing.note));
        if(!seqData->NoteEmpty(pos)) EditVelocity(xy, pos);
    }

    void ResetStepEditing()
    {
      if(!stepEditing.editing) return;
      if(stepEditing.edited) seqData->EnableTempSnapshot();
      SaveVelocity(stepEditing.note);
      stepEditing = StepEditing();
      Device::AnalogInput::DisableDial();
      seqData->Comp_EndEditing();
      seqData->Pick_EndEditing();
    }

    void ResetEditBlock()
    {
        editBlock = EditBlock();
    }

  };
}