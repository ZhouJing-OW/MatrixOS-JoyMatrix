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
    uint8_t renderFrom      = 0;
    uint8_t seqHeight       = 1;
    uint8_t scrollPos       = 0;
    uint8_t scrollMax       = 1;
    int16_t editingStep     = -1;
    Point   editingPoint    = Point(0xFFF, 0xFFF);
    PadType lastPadType     = PIANO_PAD;
    uint8_t editingNote     = 255;
    SEQ_Pos editingPos      = SEQ_Pos(channel, clipNum, 0, 0);
    int16_t velocityEdit    = -1;
    KnobConfig velocityKnob = {.lock = true, .data{.varPtr = &velocityEdit}, .min = 1, .max = 127, .def = 127, .color = COLOR_GREEN};
    std::bitset<BAR_MAX * STEP_MAX> gateMap;
    
    bool fullScreen = false;
    bool singleNoteMode = false;
    bool velocityEditing = false;

    const Color seqColor[2] = {COLOR_GREEN, COLOR_LIME};  // mono, poly
    const Color backColor[3] = {((Color)COLOR_BLUE).Scale(8), ((Color)COLOR_BLUE).Rotate(-10).Scale(8), ((Color)COLOR_AZURE).Scale(8)};  // normal, single, edit
    const Color invalidColor = COLOR_BLANK;
    const Color noBarColor   = Color(COLOR_RED).Scale(8);
    const Point barPos = Point(6, 0);
    Point seqPos = Point(0, 1);

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
      seqHeight = 4;
      ResetUI();
    }

    virtual void HalfScreen()
    {
      dimension = Dimension(16, 2);
      seqPos = Point(0, 1);
      seqHeight = 1;
      ResetUI();
    }

    virtual void ResetUI()
    {
      mode = SEQ_NORMAL;
      renderFrom = 0;
      seqData->Capture_EndEditing();
      ResetEditing();
    }

    virtual bool Render(Point origin) 
    {
      channel = MatrixOS::UserVar::global_channel;
      StateCheck(origin);
      
      BarRender(origin + barPos);
      SeqRender(origin + seqPos);
      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
    {
      if(xy.y > 0 || seqHeight > 1)
        return SeqKeyEvent(xy, seqPos, keyInfo);
      else if(xy.x >= 6 && xy.x < 10 && xy.y == 0)
        return BarKeyEvent(xy, barPos, keyInfo);
      return false;
    }

    void StateCheck(Point origin)
    {
      if (channel != channelPrv)                              // Check channel change
      {
        channelPrv = channel;
        ResetEditing();
        seqData->Capture_EndEditing();
        renderFrom = 0;
      }

      PadType padTypeNow = (PadType)channelConfig->padType[channel];
      if(padTypeNow != lastPadType)                           // Check pad type change
      {
        lastPadType = padTypeNow;
        if(padTypeNow == DRUM_PAD) singleNoteMode = true;
        else singleNoteMode = false;
      }

      if(editingPoint != Point(0xFFF, 0xFFF))                 // Check editing point button release
      {
        if(!Device::KeyPad::GetKey(Device::KeyPad::XY2ID(editingPoint - origin))->active())
        {
          MLOGD("SEQ", "Reset not in the normal way");
          ResetEditing();
          seqData->Capture_ChangeState(CAP_NORMAL);
        }
      }

      if(editingNote <= 127 && editingNote != channelConfig->activeNote[channel])   // Check Editing note change
        ResetEditing();

      SaveVelocity();                                                               // Check velocity edited.
    }

    void SeqRender(Point origin)
    {
      SEQ_Clip* clip = seqData->Clip(channel, clipNum);
      for(uint8_t x = 0; x < dimension.x; x++) {
        for(uint8_t y = 0; y < seqHeight; y++) {
          Point xy = origin + Point(x, y);
          uint8_t stepNum = x + y * dimension.x + renderFrom;
          bool playing = false;  // stepNum == sequencer->playHead[channel] && transportState.play;
          if (stepNum >= clip->barMax * STEP_MAX) 
          {
            MatrixOS::LED::SetColor(xy, COLOR_BLANK);
            continue;
          }
          
          SEQ_Pos pos = SEQ_Pos(channel, clipNum, stepNum / STEP_MAX, stepNum % STEP_MAX);
          if(stepNum % STEP_MAX >= clip->barStepMax)
            MatrixOS::LED::SetColor(xy, invalidColor);
          else if (editingStep >= 0 && editingStep == stepNum)   // renderEditing
          {
            Color thisColor = COLOR_WHITE;
            uint8_t velocity = seqData->GetVelocity(pos, editingNote);
            uint8_t scale =  velocity * 239 / 127 + 16;
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          }
          else if(editingStep >= 0 && gateMap.test(stepNum))
          {
            bool hasNote = seqData->FindNote(pos, editingNote);
            Color thisColor = playing ? COLOR_WHITE : seqColor[hasNote];
            MatrixOS::LED::SetColor(xy, thisColor.Scale(16));
          }
          else if(singleNoteMode ? seqData->FindNote(pos, channelConfig->activeNote[channel]) : !seqData->NoteEmpty(pos))
          {
            bool poly = singleNoteMode ? false : seqData->NoteCount(pos) > 1;
            Color thisColor = playing ? COLOR_WHITE : seqColor[poly];
            uint8_t velocity = seqData->GetVelocity(pos, editingNote);
            uint8_t scale =  velocity * 239 / 127 + 16;
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          }
          else
          {
            Color blinkColor = Color(backColor[singleNoteMode ? seqData->NoteEmpty(pos) : 0]).Blink_Color(mode == SEQ_CLIP_EDIT, backColor[2]);
            Color thisColor = playing ? ((Color)COLOR_WHITE).ToLowBrightness() : blinkColor;
            MatrixOS::LED::SetColor(xy, thisColor);            
          }
        }
      }
    }

    bool SeqKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      uint8_t stepNum = ui.x + ui.y * dimension.x + renderFrom;
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
              seqData->Capture_SaveEdited(pos);
              return true;
            }

            seqData->Capture_SaveNormal(pos);
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

    void BarRender(Point origin)
    {
      for (uint8_t x = 0; x < BAR_MAX; x++)
      {
        bool hasNote = seqData->FindNote(channel, clipNum, x);
        Color thisColor = x >= seqData->Clip(channel, clipNum)->barMax ? noBarColor : Color(COLOR_AZURE).ToLowBrightness(hasNote);
        if (x == barNum) thisColor = COLOR_WHITE;
        MatrixOS::LED::SetColor(origin + Point(x, 0), thisColor.Blink_Color(mode == SEQ_CLIP_EDIT, thisColor.Rotate(30)));
      }
    }

    bool BarKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      SEQ_Clip* clip = seqData->Clip(channel, clipNum);
      if(ui.x >= BAR_MAX) return false;

      switch(mode)
      {
        case SEQ_CLIP_EDIT :
          if(keyInfo->state == RELEASED)
          {
            uint8_t currentBar = ui.x;
            if(currentBar == editBar)
            {
              editBar = -1;
              ChangeUIMode(modePrv);
              return true;
            }
            if(editBar < clip->barMax)
            {
              seqData->CopyBar(SEQ_Pos(channel, clipNum, editBar, 0), SEQ_Pos(channel, clipNum, currentBar, 0));
              if (ui.x >= clip->barMax) clip->barMax = ui.x + 1;
            }
          }
          break;
        default:
          if(keyInfo->state == RELEASED && !keyInfo->hold)
          {
            if(ui.x < clip->barMax)
            {
              barNum = ui.x;
              renderFrom = ui.x * STEP_MAX;
              return true;
            }
          }
          if(keyInfo->state == HOLD)
          {
            if(Device::KeyPad::Shift())
            {
              clip->barMax = ui.x + 1;
              return true;
            }
            if(ui.x >= clip->barMax) 
            {
              clip->barMax = ui.x + 1;
              return true;
            }
            editBar = ui.x;
            ChangeUIMode(SEQ_CLIP_EDIT);
            return true;
          }
      }
      return false;
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
      if(!Device::AnalogInput::GetDialPtr() && velocityEdit > 0)
      {
        seqData->SetVelocity(editingPos, velocityEdit, editingNote);
        seqData->Capture_CopyStepNotes(editingPos);
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
      gateMap = GateMap(seqData->GetGate(pos, editingNote));
    }

    void ResetEditing()
    {
      editingPoint = Point(0xFFF,0xFFF);
      editingStep = -1;
      editingNote = singleNoteMode ? channelConfig->activeNote[channel] : 255;
    }
  };
}