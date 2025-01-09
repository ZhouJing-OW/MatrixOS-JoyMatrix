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
    enum EditState : uint8_t {
        EDIT_NONE,      // 未进入编辑状态
        COPY_BAR,       // bar复制状态
        COPY_STEP,      // step复制状态
        LOOP_BAR,       // bar loop状态
    };

    //-----------------------------------NORMAL-----------------------------------//

    const Color stepColor[2]          ={Color(LAWN_LS),             Color(LAWN)};                                    // poly, mono
    const Color backColor[3]          ={Color(BLANK),               Color(BLUE).Scale(8),    Color(CYAN).Scale(8)};  // invalid Step, poly track, mono track
    const Color barColor[3]           ={Color(DEEPSKY).Scale(8),    Color(BLUE_LS),          Color(BLUE)};           // noBar, hasBar, hasNote
    const Color playHeadColor[2]      ={Color(GREEN),               Color(ORANGE)};                                  // play, record

          Point     seqPos                = Point(0, 1);                  Dimension seqArea               = Dimension(16, 1);
    const Point     barPos                = Point(0, 0);            const Dimension barArea               = Dimension(BAR_MAX, 1);
    
    const Point     tripletBarPos         = Point(2, 0);          
    const Dimension tripletBarArea        = Dimension(12, 2);

    // const Color     tripletColor          = Color(VIOLET);
    // const Point     tripletBtnPos         = Point(1, 0);
    // const char      tripletName[8]        = "Triplet";
    //       BtnFunc   TripletBtn            = [&]()->void { ChangeUIMode(TRIPLET);};

    const Color     settingBtnColor       = Color(VIOLET);
    const char      settingBtnName[8]     = "Setting";
          BtnFunc   settingBtn            = [&]()->void { ChangeUIMode(SETTING);};


    //-----------------------------------EDITING------------------------------------//
    
    const Color     OffsetColor           = Color(YELLOW);
          Point     OffsetLPos1()         { return Point(0,  editing.point.y > 0 ? editing.point.y - 1 : 1);}
          Point     OffsetLPos2()         { return Point(14, editing.point.y > 0 ? editing.point.y - 1 : 1);}
          BtnFunc   OffsetLBtn            = [&]()->void { seqData->SetOffset(editing.pos, std::max(seqData->GetOffset(editing.pos) - 1 , -5));};

          Point     OffsetRPos1()         { return Point(1,  editing.point.y > 0 ? editing.point.y - 1 : 1);}
          Point     OffsetRPos2()         { return Point(15, editing.point.y > 0 ? editing.point.y - 1 : 1);}
          BtnFunc   OffsetRBtn            = [&]()->void { seqData->SetOffset(editing.pos, std::min(seqData->GetOffset(editing.pos) + 1 , 5)); };

    //-----------------------------------SETTING------------------------------------//

    const Color settingColor[4]       = {GREEN,      LAWN,       TURQUOISE,  BLUE};
    const Color setLabelColor[4]      = {VIOLET,     VIOLET,     VIOLET,     VIOLET};
    const char  settingName[4][9]     = {"Speed",    "Gate",     "Quantize", "Step/bar"};
    const Color speedColor[10]        = {GREEN_HL,   GREEN_HL,   GREEN_HL,   GREEN_HL,   GREEN,      GREEN_HL,   GREEN_HL,   GREEN_HL,   GREEN_HL,   GREEN_HL };
    const char  speedName[10][5]      = {"4x",       "3x",       "2x",       "1.5x",     "1x",       "2/3",      "1/2",      "1/4",      "1/8",      "1/16" };

    const Point     settingLabelPos       = Point(6, 0);            const Dimension settingLabelArea      = Dimension(4, 1);
    const Point     speedSetPos           = Point(3, 1);            const Dimension speedSetArea          = Dimension(10, 1);
    const Point     gateSetPos            = Point(3, 1);            const Dimension gateSetArea           = Dimension(10, 1);
    const Point     quantizeSetPos        = Point(2, 1);            const Dimension quantizeSetArea       = Dimension(11, 1);
    const Point     stepSetPos            = Point(0, 1);            const Dimension stepSetArea           = Dimension(16, 1);

    const Color     backBtnColor          = Color(BLUE).DimIfNot();
    const Point     backBtnPos            = Point(15, 0);
    const char      backBtnName[5]        = "Back";
          BtnFunc   backBtn               = [&]()->void { ChangeUIMode(NORMAL);};

    //-----------------------------------COMPONENT-----------------------------------//

    const Color compColor[8]          = {TURQUOISE, CYAN,      DEEPSKY,   MAROON,    PURPLE,    VIOLET,    GOLD,      YELLOW};
    const Color compLabelColor[8]     = {TURQUOISE, TURQUOISE, TURQUOISE, TURQUOISE, PURPLE,    PURPLE,    GOLD,      YELLOW};
    const char  compName[8][9]        = {"Cycle"  , "Retrig",  "Chance",  "Flam",    "OCTAVE",  "PITCH",   "STURM",   "ARP"};

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
    uint8_t     compLabel     = 0; 
    PadType     lastPadType   = PIANO_PAD;
    bool        monoMode      = false;

    KnobConfig velocityKnob   = {.lock = true, .data{.varPtr = &editing.velocity}, .min = 1, .max = 127, .def = 127, .color = Color(LAWN)  };

    KnobConfig speedKnob      = {.lock = true, .data{.varPtr = nullptr},           .min = 0, .max = 9,   .def = 4,   .color = settingColor[0] };
    KnobConfig gateKnob       = {.lock = true, .data{.varPtr = nullptr},           .min = 10,.max = 100, .def = 50,  .color = settingColor[1] };
    KnobConfig QuantizeKnob   = {.lock = true, .data{.varPtr = nullptr},           .min = 0, .max = 100, .def = 0,   .color = settingColor[2] };
    KnobConfig stepKnob       = {.lock = true, .data{.varPtr = nullptr},           .min = 8, .max = 16,  .def = 16,  .color = settingColor[3] };

    std::vector<KnobConfig*> settingKnob= {nullptr, nullptr, &speedKnob, &gateKnob, &QuantizeKnob, &stepKnob, nullptr, nullptr};

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

    const Point     copyBtnPos           = Point(0, 2);     // 复制按钮位置
    const Color     copyBtnColor         = Color(CYAN);     // 复制按钮颜色
    const char      copyBtnName[5]       = "Copy";

    const Point     deleteBtnPos         = Point(1, 2);     // 删除按钮位置
    const Color     deleteBtnColor       = Color(RED);      // 删除按钮颜色
    const char      deleteBtnName[7]     = "Delete";

  public : SequencerUI() {
    channel = MatrixOS::UserVar::global_channel;
    channelPrv = 255;
    if (channelConfig->padType[channel] == DRUM_PAD)
      monoMode = true;
    else
      monoMode = false;
  }

    ~SequencerUI() { seqData->Pick_EndEditing();}

    virtual void SetNull() {}
    virtual bool SetKnobPtr() { knobPtr.clear(); return false; }
    virtual Dimension GetSize() { return dimension; }

    void ChangeUIMode(SeqUIMode mode) 
    {
      switch(mode)
      { 
        case NORMAL: break;
        case TRIPLET: 
          break;
        case SETTING: 
          settingLabel = 0;
          break;
        case COMPONENT:
          compLabel = 0;
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
          if(!fullScreen)
          { 
            BarRender(origin, barPos);
            SeqRender(origin, seqPos);
          } 
          else 
          {
            BarRender(origin, barPos);                    // 第0行显示bar
            SeqRender(origin, Point(0, 1));               // 第1行显示序列器

            // 复制按钮显示逻辑
            Color copyColor = editBlock.copyKeyHeld ? Color(WHITE) : copyBtnColor;
            ButtonRender(origin, copyBtnPos, copyColor);
            
            // 删除按钮显示逻辑
            Color deleteColor = editBlock.deleteKeyHeld ? Color(WHITE) : deleteBtnColor;
            ButtonRender(origin, deleteBtnPos, deleteColor);
          }
          break;
        case TRIPLET:  
          break;
        case SETTING:
          LabelRender (settingLabelArea, origin, settingLabelPos, settingName, setLabelColor, settingLabel, true);
          ButtonRender(origin, backBtnPos, backBtnColor);
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
              SeqRender(origin, seqPos);
              break;
          }
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
            
            if(editBlock.copyKeyHeld && editBlock.state == EDIT_NONE)
            {
              if(InArea(xy, Point(0, 1), Dimension(16, 1)) && keyInfo->state == PRESSED)
              {
                editBlock.state = COPY_STEP;
                Point ui = xy - Point(0, 1);
                uint8_t localStep = ui.x + ui.y * dimension.x;
                uint8_t globalStep = localStep + barNum * STEP_MAX;
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
                return ValueBarKeyEvent(gateSetArea, xy, gateSetPos, keyInfo, settingColor[1], clip->tair, 10, 100);
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
        case COMPONENT:
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

      if (sequencer->recording) {
        barNum = sequencer->playHead / STEP_MAX;
      }

      PadType padTypeNow = (PadType)channelConfig->padType[channel];
      if(padTypeNow != lastPadType)                                                        // Check pad type change
      {
        lastPadType = padTypeNow;
        if(padTypeNow == DRUM_PAD) monoMode = true;
        else monoMode = false;
      }
      
      SaveVelocity(editing.note);                                                          // Check velocity edited.
    
      if (seqData->Comp_InEditing())                                                     // Check component editing
        ChangeUIMode(COMPONENT);
      else if (mode == COMPONENT)
        ChangeUIMode(NORMAL);

      //--------------------------------------EDITING--------------------------------------//

      if(editing.point == Point(0xFFF, 0xFFF))  
      {
        return; 
      }
                  
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

    void ResetUI()
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
                    if (stepNum >= start && stepNum <= end) {
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
            MatrixOS::LED::SetColor(xy, thisColor);
          }
        }
      }
    }

    bool SeqKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
        Point ui = xy - offset;
        uint8_t localStep = ui.x + ui.y * dimension.x;
        uint8_t globalStep = localStep + barNum * STEP_MAX;
        SEQ_Pos pos = SEQ_Pos(channel, clipNum, barNum, localStep);

        // 首先检查步进是否有效
        if(localStep >= clip->barStepMax || localStep / STEP_MAX >= clip->barMax) return false;

        // 删除操作应该优先于其他操作
        if (editBlock.deleteKeyHeld) {
            Point ui = xy - offset;
            editBlock.deleteStepTarget = ui;  // 使用 deleteStepTarget
            
            if (keyInfo->state == RELEASED && !keyInfo->hold) {
                if(localStep >= clip->barStepMax) return false;

                if (monoMode) {
                    uint8_t activeNote = channelConfig->activeNote[channel];
                    if (seqData->FindNote(pos, activeNote)) {
                        seqData->DeleteNote(pos, activeNote);
                    }
                } else {
                    seqData->ClearNote(pos);
                }
                editBlock.deleteStepTarget = Point(-1, -1);  // 重置删除目标位置
                return true;
            }
            return false;  // 删除键按住时阻止其他操作
        }

        // 复制操作
        if (editBlock.copyKeyHeld && editBlock.state == COPY_STEP) {
            if(localStep >= clip->barStepMax) return false;

            if (keyInfo->state == PRESSED) {
                int16_t start = std::min<int16_t>(editBlock.stepStart, editBlock.stepEnd);
                int16_t end = std::max<int16_t>(editBlock.stepStart, editBlock.stepEnd);
                int16_t range = end - start + 1;
                int16_t dstStep = globalStep;
                
                // 计算最大可复制步数，确保类型一致
                int16_t maxSteps = static_cast<int16_t>(BAR_MAX * STEP_MAX - dstStep);
                int16_t copyRange = std::min<int16_t>(range, maxSteps);
                
                if (copyRange > 0) {
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

        // 编辑操作
        if (mode == SETTING)
        {
          if(keyInfo->state == RELEASED)
          {
            if (localStep % STEP_MAX >= 2) clip->barStepMax = localStep % STEP_MAX + 1;
            else clip->barStepMax = 3;
            if (editing.step >= 0) GateRenderMap(seqData->GetGate(editing.pos, editing.note));
            return true;
          }
          if(keyInfo->state == HOLD)
          {
            if (xy.x >= 2)
              MatrixOS::UIInterface::TextScroll(std::to_string(xy.x + 1) + "/Bar", Color(BLUE));
            else
              MatrixOS::UIInterface::TextScroll("3/Bar", Color(BLUE));
            return true;
          }
          return true;
        }

        // 普通音符编辑
        switch(monoMode)
        {
          case true:

            if(editing.step == localStep && keyInfo->state == RELEASED)
            {
              ResetEditing();
              if(!keyInfo->hold)
                seqData->Pick_SaveSingle(pos);
              else
                seqData->Pick_SaveHold(pos);
              return true;
            }

            if(keyInfo->state == PRESSED)
            {
              if (editing.step >= 0)
              {
                if(seqData->FindNote(editing.pos, editing.note))
                  SetGate(localStep);
                return true;
              }
              // if(!seqData->NoteEmpty(pos)) EditVelocity(xy, pos);
              seqData->Pick_Editing(pos);
              SetEditing(pos, xy, localStep);
              return true;
            }
            return false;
        
          case false:

            if(editing.step == localStep && keyInfo->state == RELEASED)
            {
              ResetEditing();
              if(keyInfo->shortHold)
              {
                ResetEditing();
                seqData->Pick_SaveHold(pos);
                return true;
              }

              seqData->Pick_SaveClick(pos);
              ResetEditing();
              return true;
            }

            if (keyInfo->state == PRESSED)
            {
              if (editing.step >= 0)
              {
                if(seqData->FindNote(editing.pos, editing.note))
                  SetGate(localStep);
                return true;
              }
              seqData->Pick_Editing(pos);
              SetEditing(pos, xy, localStep);
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
            
            // Loop 设置状态下的显示逻辑
            if (editBlock.state == LOOP_BAR) {
                if (clip->HasLoop() && x >= clip->loopStart && x <= clip->loopEnd) {
                    // loop 范围内的 bar 正常显示
                    thisColor = x == barNum ? Color(WHITE) : barColor[hasBar + hasBar * hasNote];
                } else {
                    // loop 范围外的 bar 闪烁
                    Color baseColor = barColor[hasBar + hasBar * hasNote];
                    thisColor = x == barNum ? Color(WHITE) : baseColor.Blink_Color(true, Color(BLANK));
                }
            }
            // 在复制或删除状态下显示全范围
            else if (editBlock.copyKeyHeld || editBlock.deleteKeyHeld) {
                thisColor = x == barNum ? Color(WHITE) : barColor[hasBar + hasBar * hasNote];
            }
            // 正常状态下根据 loop 显示
            else if (clip->HasLoop()) {
                if (x >= clip->loopStart && x <= clip->loopEnd) {
                    thisColor = x == barNum ? Color(WHITE) : barColor[hasBar + hasBar * hasNote];
                } else {
                    if (x < clip->barMax) {
                        thisColor = Color(BLUE).Scale(8);
                    } else {
                        thisColor = Color(VIOLET).Scale(8);
                    }
                }
            } else {
                thisColor = x == barNum ? Color(WHITE) : barColor[hasBar + hasBar * hasNote];
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
            editBlock.deleteBarTarget = Point(ui.x, -1);  // 使用 deleteBarTarget
            
            if (keyInfo->state == RELEASED && !keyInfo->hold) {
                if (thisBar < clip->barMax) {
                    seqData->ClearBar(SEQ_Pos(channel, clipNum, thisBar, 0));
                    if (thisBar >= clip->barMax - 1 && clip->barMax > 1) 
                    {
                      clip->barMax--;
                      if (barNum >= clip->barMax) {
                          barNum = clip->barMax - 1;
                      }
                    }
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
                        clip->loopStart = start;
                        clip->loopEnd = end;
                        
                        if (end >= clip->barMax) {
                            clip->barMax = end + 1;
                        }

                        if (barNum < start || barNum > end) {
                            barNum = start;
                        }
                    }
                }
                else if (editBlock.barKeyCount == 1) {
                    if (clip->HasLoop()) {
                        clip->ClearLoop();
                    } else {
                        clip->loopStart = thisBar;
                        clip->loopEnd = thisBar;
                        
                        if (thisBar >= clip->barMax) {
                            clip->barMax = thisBar + 1;
                        }
                        barNum = thisBar;
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
                        clip->barMax = thisBar + 1;
                    }
                    barNum = thisBar;
                }
            }
        }
        return true;
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
          seqData->Pick_Update(editing.pos);
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
      if(EndEditing) 
      {
        seqData->Comp_EndEditing();
        seqData->Pick_EndEditing();
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

    void ResetEditBlock()
    {
        editBlock.state = EDIT_NONE;
        editBlock.barStart = -1;
        editBlock.barEnd = -1;
        editBlock.stepStart = -1;
        editBlock.stepEnd = -1;
        editBlock.loopEditBar = -1;  // 重置 loopEditBar
        editBlock.copyKeyHeld = false;
        editBlock.deleteKeyHeld = false;
        editBlock.deleteBarTarget = Point(-1, -1);
        editBlock.deleteStepTarget = Point(-1, -1);

        if (clip->HasLoop()) {
            if (barNum < clip->loopStart || barNum > clip->loopEnd) {
                barNum = clip->loopStart;
            }
        }
    }
  };
}