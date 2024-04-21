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
    SeqUIMode mode = SEQ_NORMAL;
    Sequencer* sequencer;
    uint8_t clipNum         = 0;
    uint8_t renderFrom      = 0;
    uint8_t seqHeight       = 1;
    uint8_t scrollPos       = 0;
    uint8_t scrollMax       = 1;
    int16_t editingStep      = -1;
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
    const Color backColor[2] = {((Color)COLOR_BLUE).Scale(8), ((Color)COLOR_BLUE).Rotate(-10).Scale(8)};  // normal, single
    const Color invalidColor = COLOR_BLANK;  //((Color)COLOR_RED).ToLowBrightness();
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

      SeqRender(origin + seqPos);
      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
    {
      return SeqKeyEvent(xy, seqPos, keyInfo);
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
          ResetEditing();
          seqData->Capture_ChangeState(CAP_NORMAL);
        }
      }

      if(editingNote <= 127 && editingNote != channelConfig->activeNote[channel])   // Check Editing note change
        ResetEditing();

      SaveVelocity(origin);                                   // Check velocity edited.
    }

    void SeqRender(Point origin)
    {
      for(uint8_t x = 0; x < dimension.x; x++) {
        for(uint8_t y = 0; y < seqHeight; y++) {
          Point xy = origin + Point(x, y);
          uint8_t stepNum = x + y * dimension.x + renderFrom;
          bool playing = false;  // stepNum == sequencer->playHead[channel] && transportState.play;
          bool editing = editingStep == stepNum;
          if (stepNum >= STEP_MAX) 
          {
            MatrixOS::LED::SetColor(xy, COLOR_BLANK);
            continue;
          }
          
          SEQ_Pos pos = SEQ_Pos(channel, clipNum, stepNum / STEP_MAX, stepNum % STEP_MAX);

          if ((editingStep >= 0 && editingStep == stepNum) || (editingStep < 0 && singleNoteMode ? seqData->FindNote(pos, channelConfig->activeNote[channel]) : !seqData->NoteEmpty(pos)))
          {
            bool poly = singleNoteMode ? false : seqData->NoteCount(pos) > 1;
            Color thisColor = (playing || editing) ? COLOR_WHITE : seqColor[poly];
            uint8_t velocity = seqData->GetVelocity(pos, singleNoteMode ? channelConfig->activeNote[channel] : 255);
            uint8_t scale =  velocity * 239 / 127 + 16;
            MatrixOS::LED::SetColor(xy, thisColor.Scale(scale));
          }
          else if(editingStep >= 0 && gateMap.test(stepNum))
          {
            bool poly = singleNoteMode ? false : seqData->NoteCount(pos) > 1;
            Color thisColor = (playing || editing) ? COLOR_WHITE : seqColor[poly];
            MatrixOS::LED::SetColor(xy, thisColor.Scale(16));
          }
          else
          {
            Color thisColor = (playing || editing) ? ((Color)COLOR_WHITE).ToLowBrightness() : backColor[singleNoteMode ? seqData->NoteEmpty(pos) : 0];
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
      if(!clip) return false;
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
            
            uint8_t note = channelConfig->activeNote[channel];
            SetEditing(pos, xy, stepNum, note);
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
          return false;
      }
    }

    void SetGate(uint8_t stepNum)
    {
      uint8_t invaildStep = 16 - seqData->Clip(channel, clipNum)->barStepMax;
      uint8_t realEditingNum = editingStep - (editingStep / STEP_MAX - 1 ) * invaildStep;
      uint8_t realStepNum = stepNum + (editingStep > stepNum ? seqData->Clip(channel, clipNum)->barMax * STEP_MAX : 0);
      realStepNum = realStepNum - (realStepNum / STEP_MAX - 1 ) * invaildStep;
      uint8_t gateLength = realStepNum - realEditingNum;
      uint8_t currentLength = seqData->GetGate(editingPos, singleNoteMode ? channelConfig->activeNote[channel] : 255);
      seqData->SetGate(editingPos, gateLength == currentLength ? 0 : gateLength, singleNoteMode ? channelConfig->activeNote[channel] : 255);
      gateMap = GateMap(seqData->GetGate(editingPos, singleNoteMode ? channelConfig->activeNote[channel] : 255));
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
        if (edit >= stepAll) edit = 0;
        gateBit.set(edit);
      }
      return gateBit;
    }

    void SaveVelocity(Point origin)
    {
      if(!Device::AnalogInput::GetDialPtr() && velocityEdit > 0)
      {
        seqData->SetVelocity(editingPos, velocityEdit, singleNoteMode ? channelConfig->activeNote[channel] : 255);
        velocityEdit = -1;
      }
    }

    void EditVelocity(Point xy, SEQ_Pos position)
    {
      velocityEdit = seqData->GetVelocity(editingPos, singleNoteMode ? channelConfig->activeNote[channel] : 255);
      Device::AnalogInput::UseDial(xy, &velocityKnob);
    }

    void SetEditing(SEQ_Pos pos, Point xy, uint8_t stepNum, uint8_t note = 255)
    { 
      if(!seqData->NoteEmpty(pos)) EditVelocity(xy, pos);
      editingPoint = xy;
      editingPos = pos;
      editingStep = stepNum;
      editingNote = note;
      gateMap = GateMap(seqData->GetGate(pos, note));
    }

    void ResetEditing()
    {
      editingPoint = Point(0xFFF,0xFFF);
      editingStep = -1;
      editingNote = 255;
    }
  };
}