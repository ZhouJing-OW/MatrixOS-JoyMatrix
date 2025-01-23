#pragma once
#include "MidiCenter.h"
#include "SeqData.h"
#include "Sequencer.h"
#include "NodeUI.h"

namespace MatrixOS::MidiCenter
{
  extern SEQ_DataStore*     seqData;
  class ClipSelector : public UIComponent
  {
  public:
    Dimension dimension;
    int8_t renderFrom = 0;
    uint8_t scrollMax = 4;
    int8_t copyFrom_ch, copyFrom_clip = -1;

    ClipSelector(Dimension dimension = Dimension(16, 4))
    {
      this->dimension = dimension;
    }

    ~ClipSelector() { Device::AnalogInput::DisableUpDown(); }

    virtual Dimension GetSize() { return dimension; }

    virtual bool Render(Point origin)
    {
      Device::AnalogInput::SetUpDown(&renderFrom, scrollMax);

      for (uint8_t x = 0; x < dimension.x; x++) {
        uint8_t editingClip = seqData->EditingClip(x);
        for (uint8_t y = 0; y < dimension.x; y++) {
          uint8_t thisClip = renderFrom + y;
          if(thisClip >= CLIP_MAX)
          {
            MatrixOS::LED::SetColor(origin + Point(x, y), Color(BLANK));
            continue;
          }
          
          Point xy = origin + Point(x, y);
          if(!seqData->Clip(x, thisClip)->Empty())
          {
            Color thisColor  = channelConfig->color[x];
            Color blinkColor = x == MatrixOS::UserVar::global_channel ? Color(WHITE) : thisColor.Mix(WHITE);
            if(thisClip == editingClip)
              MatrixOS::LED::SetColor(xy, thisColor.Blink_Color(true, blinkColor));
            else
              MatrixOS::LED::SetColor(xy, thisColor.DimIfNot());
          }
          else
          {
            Color backColor = thisClip > 3 ? Color(CYAN).Scale(8) : Color(BLUE).Scale(8);
            Color blinkColor = x == MatrixOS::UserVar::global_channel ? Color(WHITE) : Color(WHITE).Scale(8);
            MatrixOS::LED::SetColor(xy, (backColor.Blink_Color(thisClip == editingClip, blinkColor)));
          }
        }
      }
        return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
    {
      uint8_t thisClip = xy.y + renderFrom;
      if(keyInfo->state == RELEASED)
      {
        if(copyFrom_clip < 0)
        {
          if(!keyInfo->hold) 
          {
            seqData->SetEditingClip(xy.x, thisClip);
            MatrixOS::UserVar::global_channel = xy.x;
            Send_On(SEND_CC, xy.x, channelConfig->selectCC, 255);
            Send_Off(SEND_CC, xy.x, channelConfig->selectCC);
          }
        }
        else if(copyFrom_ch == xy.x && copyFrom_clip == thisClip)
          copyFrom_clip = -1;
        else
          seqData->CopyClip(copyFrom_ch, copyFrom_clip, xy.x, thisClip);
      }
      if(keyInfo->state == HOLD)
      {
        copyFrom_ch = xy.x;
        copyFrom_clip = thisClip;
        return true;
      }

      return false;
    }

  };

};
