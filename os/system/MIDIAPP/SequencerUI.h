#pragma once
#include "MidiCenter.h"
#include "SeqData.h"
#include "Sequencer.h"
#include "NodeUI.h"

namespace MatrixOS::MidiCenter
{
  extern SEQ_DataStore*     seqData;

  class SequencerUI : public NodeUI {

    enum SeqUIMode : uint8_t {SEQ_NORMAL, SEQ_SCROLL, SEQ_SINGLE_NOTE, SEQ_TRIPLET, SEQ_CLIP_EDIT};

  public:
    SeqUIMode mode          = SEQ_NORMAL;
    SeqUIMode modePrv       = SEQ_NORMAL;
    Sequencer* sequencer;
    uint8_t clipNum         = 0;
    int8_t  editBar         = -1;
    uint8_t barNum          = 0;       
    uint8_t scrollPos       = 0;
    uint8_t scrollMax       = 1;
    int16_t editingStep     = -1;
    Point   editingPoint    = Point(0xFFF, 0xFFF);
    PadType lastPadType     = PIANO_PAD;
    uint8_t editingNote     = 255;
    uint8_t editingNoteCount = 0;
    SEQ_Pos editingPos      = SEQ_Pos(channel, clipNum, 0, 0);
    int16_t velocityEdit    = -1;
    KnobConfig velocityKnob = {.lock = true, .data{.varPtr = &velocityEdit}, .min = 1, .max = 127, .def = 127, .color = Color(LAWN)};
    std::bitset<BAR_MAX * STEP_MAX> gateMap;
    
    bool fullScreen = false;
    bool singleNoteMode = false;
    bool velocityEditing = false;

    const Color seqColor[2]       = {Color(LAWN), Color(GREEN)};  // mono, poly
    const Color backColor[3]      = {Color(BLUE).Scale(8), Color(BLUE).Rotate(-10).Scale(8), Color(CYAN).Scale(8)};  // normal, single, edit
    const Color invalidColor      =  Color(BLANK);
    const Color hasBarColor       =  Color(CYAN);
    const Color noBarColor        =  Color(RED).Scale(8);

          Point seqPos            = Point(0, 1);                   Dimension seqArea    = Dimension(16, 1);
    const Point barPos            = Point(6, 0);             const Dimension barArea    = Dimension(BAR_MAX, 1);
    const Point barDimBtnPos      = Point(5, 0);             const Dimension button1x1  = Dimension(1,1);
    const Point AutoGrouthBtnPos  = Point(10, 0);

    SequencerUI() 
    { 
      channel = MatrixOS::UserVar::global_channel;
      channelPrv = channel;
      if(channelConfig->padType[channel] == DRUM_PAD) singleNoteMode = true;
      else singleNoteMode = false;
    }

    ~SequencerUI() { seqData->Capture_EndEditing(); }

    virtual void SetNull() {}
    virtual bool SetKnobPtr() { knobPtr.clear(); return false; }
    virtual Dimension GetSize() { return dimension; }

    virtual void ChangeUIMode(SeqUIMode mode) 
    {
      switch(mode)
      {
        case SEQ_NORMAL: break;
        case SEQ_SCROLL: break;
        case SEQ_SINGLE_NOTE: break;
        case SEQ_TRIPLET: break;
        case SEQ_CLIP_EDIT: break;
      }
      modePrv = this->mode;
      this->mode = mode;
      ResetEditing();
      seqData->Capture_EndEditing();
    }

    virtual void FullScreen()
    {
      dimension = Dimension(16, 4);
      seqPos = Point(0, 0);
      seqArea.y = 4;
      ResetUI();
    }

    virtual void HalfScreen()
    {
      dimension = Dimension(16, 2);
      seqPos = Point(0, 1);
      seqArea.y = 1;
      ResetUI();
    }

    virtual void ResetUI()
    {
      mode = SEQ_NORMAL;
      barNum = 0;
      ResetEditing();
      seqData->Capture_EndEditing();
      
    }

    virtual bool Render(Point origin) 
    {
      channel = MatrixOS::UserVar::global_channel;
      StateCheck(origin);

      SeqRender(origin, seqPos);
      BarRender(origin, barPos);
      BarDimBtnRender(origin, barDimBtnPos);
      AutoGrouthBtnRender(origin, AutoGrouthBtnPos);
      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
    {
      if(InArea(xy, seqPos, seqArea))               return SeqKeyEvent(xy, seqPos, keyInfo);
      if(InArea(xy, barPos, barArea))               return BarKeyEvent(xy, barPos, keyInfo);
      if(InArea(xy, barDimBtnPos, button1x1))       return BarDimBtnKeyEvent(xy, barDimBtnPos, keyInfo);
      if(InArea(xy, AutoGrouthBtnPos, button1x1))   return AutoGrouthBtnKeyEvent(xy, AutoGrouthBtnPos, keyInfo);
      return false;
    }
  
  private:
    void StateCheck(Point origin)
    {
      if (channel != channelPrv)                                                           // Check channel change
      {
        channelPrv = channel;
        ResetEditing();
        seqData->Capture_EndEditing();
        barNum = 0;
      }

      PadType padTypeNow = (PadType)channelConfig->padType[channel];
      if(padTypeNow != lastPadType)                                                        // Check pad type change
      {
        lastPadType = padTypeNow;
        if(padTypeNow == DRUM_PAD) singleNoteMode = true;
        else singleNoteMode = false;
      }
      
      SaveVelocity();                                                                      // Check velocity edited.

      if(editingPoint == Point(0xFFF, 0xFFF))  return;    
                  
      if(!Device::KeyPad::GetKey(Device::KeyPad::XY2ID(editingPoint - origin))->active())  // Check editing point button release
      {
        MLOGD("SEQ", "Reset Editing");
        ResetEditing();
        seqData->Capture_ChangeState(CAP_NORMAL);
      }

      if(singleNoteMode && editingNote != channelConfig->activeNote[channel])              // Check editing note change in single note mode
        ResetEditing();
      
      if(seqData->NoteCount(editingPos) > editingNoteCount)                                // Check NoteCount change
        EditVelocity(editingPoint, editingPos);
      editingNoteCount = seqData->NoteCount(editingPos, editingNote);
      if(!editingNoteCount) Device::AnalogInput::DisableDial();
                                                                  
    }

    void SeqRender(Point origin, Point offset)
    {
      SEQ_Clip* clip = seqData->Clip(channel, clipNum);
      for(uint8_t x = 0; x < seqArea.x; x++) {
        for(uint8_t y = 0; y < seqArea.y; y++) {
          Point xy = origin + offset + Point(x, y);
          uint8_t stepNum = x + y * dimension.x + barNum * STEP_MAX;
          bool playing = false;  // stepNum == sequencer->playHead[channel] && transportState.play;
          if (stepNum >= clip->barMax * STEP_MAX) 
          {
            MatrixOS::LED::SetColor(xy, Color(BLANK));
            continue;
          }
          
          SEQ_Pos pos = SEQ_Pos(channel, clipNum, stepNum / STEP_MAX, stepNum % STEP_MAX);
          if(stepNum % STEP_MAX >= clip->barStepMax)
            MatrixOS::LED::SetColor(xy, invalidColor);
          else if (editingStep >= 0 && editingStep == stepNum)   // renderEditing
          {
            Color thisColor = Color(WHITE);
            uint8_t velocity = seqData->GetVelocity(pos, editingNote);
            uint8_t scale =  velocity * 239 / 127 + 16;
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          }
          else if(editingStep >= 0 && gateMap.test(stepNum))
          {
            bool hasNote = seqData->FindNote(pos, editingNote);
            Color thisColor = playing ? Color(WHITE) : seqColor[hasNote];
            MatrixOS::LED::SetColor(xy, thisColor.Scale(16));
          }
          else if(singleNoteMode ? seqData->FindNote(pos, channelConfig->activeNote[channel]) : !seqData->NoteEmpty(pos))
          {
            bool poly = singleNoteMode ? false : seqData->NoteCount(pos) > 1;
            Color thisColor = playing ? Color(WHITE) : seqColor[poly];
            uint8_t velocity = seqData->GetVelocity(pos, editingNote);
            uint8_t scale =  velocity * 239 / 127 + 16;
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          }
          else
          {
            Color blinkColor = Color(backColor[singleNoteMode ? seqData->NoteEmpty(pos) : 0]).Blink_Color(mode == SEQ_CLIP_EDIT, backColor[2]);
            Color thisColor = playing ? Color(WHITE).ToLowBrightness() : blinkColor;
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
      SEQ_Clip* clip = seqData->Clip(channel, clipNum);

      if (mode == SEQ_CLIP_EDIT)
      {
        if(keyInfo->state == RELEASED)
        {
          if (stepNum % STEP_MAX >= 8) clip->barStepMax = stepNum % STEP_MAX + 1;
          if (editingStep >= 0) gateMap = GateMap(seqData->GetGate(editingPos, editingNote));
          return true;
        }
        return true;
      }

      if(stepNum % STEP_MAX >= clip->barStepMax || stepNum / STEP_MAX >= clip->barMax) return false;

      switch(singleNoteMode)
      {
        case true:

          if(editingStep == stepNum && keyInfo->state == RELEASED)
          {
            ResetEditing();

            if(!keyInfo->shortHold)
            {
              seqData->Capture_SaveSingle(pos);
              return true;
            }
            return false;
          }

          if(keyInfo->state == PRESSED)
          {
            if (editingStep >= 0)
            {
              SetGate(stepNum);
              return true;
            }
            if(!seqData->NoteEmpty(pos)) EditVelocity(xy, pos);
            
            SetEditing(pos, xy, stepNum);
            return true;
          }
          return false;
      
        case false:

          if(editingStep == stepNum && keyInfo->state == RELEASED)
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
            if (editingStep >= 0)
            {
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

    void BarDimBtnRender(Point origin, Point offset)
    {
      MatrixOS::LED::SetColor(origin + offset, seqData->Clip(channel, clipNum)->barMax > 1 ? Color(RED).ToLowBrightness() : Color(BLANK));
    }

    bool BarDimBtnKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      SEQ_Clip* clip = seqData->Clip(channel, clipNum);
      if(keyInfo->state == RELEASED)
      {
        if(clip->barMax > 1) clip->barMax -= 1;
        if(barNum >= clip->barMax) barNum = clip->barMax - 1;
      }
      return true;
    }
    
    void BarRender(Point origin, Point offset)
    {
      for (uint8_t x = 0; x < BAR_MAX; x++)
      {
        bool hasNote = seqData->FindNote(channel, clipNum, x);
        Color thisColor = x >= seqData->Clip(channel, clipNum)->barMax ? noBarColor : Color(hasBarColor).ToLowBrightness(hasNote);
        if (x == barNum) thisColor = Color(WHITE);
        MatrixOS::LED::SetColor(origin + offset + Point(x, 0), thisColor.Blink_Color(mode == SEQ_CLIP_EDIT, thisColor.Rotate(30)));
      }
    }

    bool BarKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      SEQ_Clip* clip = seqData->Clip(channel, clipNum);
      if(ui.x >= BAR_MAX) return false;

      if(editBar >= 0 && keyInfo->state == RELEASED)
      {
        uint8_t thisBar = ui.x;
        
        if(thisBar == editBar)
        {
          if(!keyInfo->shortHold)
            barNum = thisBar;
          if(thisBar >= clip->barMax)
            clip->barMax = thisBar + 1;
          editBar = -1;
          return true;
        }
        if(editBar < clip->barMax)
        {
          seqData->CopyBar(SEQ_Pos(channel, clipNum, editBar, 0), SEQ_Pos(channel, clipNum, thisBar, 0));
          if (ui.x >= clip->barMax) clip->barMax = ui.x + 1;
        }
      }
      if(editBar < 0 && keyInfo->state == PRESSED)
      {
        editBar = ui.x;
        return true;
      }

      return false;
    }

    void AutoGrouthBtnRender(Point origin, Point offset)
    {
      MatrixOS::LED::SetColor(origin + offset, Color(LAWN).ToLowBrightness(transportState.autoGrouth));
    }

    bool AutoGrouthBtnKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      if(keyInfo->state == RELEASED)
        transportState.autoGrouth = !transportState.autoGrouth;
      return true;
    }
    
    void SetGate(uint8_t stepNum)
    {
      uint8_t invaildStep = STEP_MAX - seqData->Clip(channel, clipNum)->barStepMax;
      if(stepNum < editingStep) stepNum += seqData->Clip(channel, clipNum)->barMax * STEP_MAX;
      stepNum -= (stepNum / STEP_MAX - 1 ) * invaildStep;
      uint8_t editNum = editingStep - (editingStep / STEP_MAX - 1 ) * invaildStep;
      uint8_t gate = stepNum - editNum;
      uint8_t currentLength = seqData->GetGate(editingPos, editingNote);
      seqData->SetGate(editingPos, gate == currentLength ? 0 : gate, editingNote);
      gateMap = GateMap(seqData->GetGate(editingPos, editingNote));
    }

    std::bitset<BAR_MAX * STEP_MAX> GateMap(uint8_t gateLength)
    {
      uint8_t inUseStep = seqData->Clip(channel, clipNum)->barStepMax;
      uint8_t stepAll = seqData->Clip(channel, clipNum)->barMax * STEP_MAX;
      uint8_t tail = gateLength / inUseStep * STEP_MAX + gateLength % inUseStep;
      std::bitset<BAR_MAX * STEP_MAX> gateBit;
      uint8_t edit = editingStep;
      while (tail)
      {
        edit++; tail--;
        if (edit % STEP_MAX >= inUseStep) edit += STEP_MAX - edit % STEP_MAX;
        if (edit >= stepAll) edit = 0;
        gateBit.set(edit);
      }
      return gateBit;
    }

    void SaveVelocity()
    {
      if(velocityEdit > 0 && !Device::AnalogInput::GetDialPtr())
      {
        seqData->SetVelocity(editingPos, velocityEdit, editingNote);
        if(!seqData->NoteEmpty(editingPos))
          seqData->Capture_UpdateCap(editingPos);
        velocityEdit = -1;
      }
    }

    void EditVelocity(Point xy, SEQ_Pos position)
    {
      velocityEdit = seqData->GetVelocity(position, editingNote);
      Device::AnalogInput::UseDial(xy, &velocityKnob);
    }

    void SetEditing(SEQ_Pos pos, Point xy, uint8_t stepNum)
    { 
      if(!seqData->NoteEmpty(pos)) EditVelocity(xy, pos);
      editingPoint = xy;
      editingPos = pos;
      editingStep = stepNum;
      editingNote = singleNoteMode ? channelConfig->activeNote[channel] : 255;
      editingNoteCount = seqData->NoteCount(pos, editingNote);
      gateMap = GateMap(seqData->GetGate(pos, editingNote));
    }

    void ResetEditing()
    {
      editingPoint = Point(0xFFF,0xFFF);
      editingStep = -1;
      editingNote = singleNoteMode ? channelConfig->activeNote[channel] : 255;
      editingNoteCount = 0;
    }
  };
}