#pragma once
#include "FeedBackUI.h"
#include "FeedBackButtons.h"
#include "ArpeggiatorUI.h"
#include "ChorderUI.h"
#include "SequencerUI.h"
#include "sequencerUI_Clip.h"
#include "MidiCenter.h"

#include <memory>

namespace MatrixOS::MidiCenter
{
  extern MultiPad* multiPad;
  class MidiAppUI: public UIComponent
  {
  public:
    std::unique_ptr<NodeUI> ui;
    Dimension dimension;
    uint8_t channel;
    uint8_t channelPrv;
    NodeID activeUI[16];
    NodeID lastUI[16];
    int8_t largePad[16];
    bool holdFN = false;

    enum MultiPadType                       { FULL_PAD,         HALF_PAD,           MINI_PAD };
    const Point MultiPadOrigin[3] =         {Point(0, 0),       Point(0, 2),        Point(4, 2)};
    const Dimension MultiPadDimension[3] =  {Dimension(16, 4),  Dimension(16, 2),   Dimension(8, 2)};

    MidiAppUI(Dimension dimension = Dimension(16, 4))
    {
      this->dimension = dimension;
      memset(activeUI,  NODE_FEEDBACK,   sizeof(activeUI));
      memset(lastUI,    NODE_NONE,  sizeof(lastUI));
      memset(largePad,  false,      sizeof(largePad));
      SetUI(NODE_FEEDBACK);
    }

    ~MidiAppUI() { Device::AnalogInput::DisableUpDown(); }

    virtual Dimension GetSize() { return dimension; }
    UIComponent* GetUI() { return ui.get(); }

    void SetUI(NodeID nodeID = NODE_NONE) 
    {
      MatrixOS::KnobCenter::DisableExtraPage();
      if(nodeID != NODE_NONE) largePad[channel] = false;
      activeUI[channel] = nodeID;
      switch (nodeID)
      {
        case NODE_SEQ:
          ui = std::make_unique<SequencerUI>(); 
          break;
        case NODE_ARP:
          ui = std::make_unique<ArpeggiatorUI>(); 
          break;
        case NODE_CHORD:
          ui = std::make_unique<ChorderUI>(); 
          break;
        case NODE_FEEDBACK:
          ui = std::make_unique<FeedBackUI>();
          break;
        default:
          ui = nullptr;
          break;
      }
      if(ui != nullptr) ui->VarGet();
    }

    inline void NodeOn(NodeID nodeID) { 
      if(activeUI[channel] == nodeID) ui->On();
      else NodeInsert(channel, nodeID);
    }

    inline void NodeOff(NodeID nodeID) { 
      if(activeUI[channel] == nodeID) ui->Off();
      else NodeDelete(channel, nodeID);
    }

    virtual bool Render(Point origin) {
      channel = MatrixOS::UserVar::global_channel;
      if(channelPrv != channel)
      {
        SetUI(activeUI[channel]);
        channelPrv = channel;
      }

      if (Device::KeyPad::fnState.active() && largePad[channel])                    // check FN state
      { largePad[channel] = false; holdFN = true;  } 
      if (!Device::KeyPad::fnState.active() && holdFN)
      { largePad[channel] = true;  holdFN = false; }
      
      if(ui == nullptr && Device::AnalogInput::GetUDPtr() != &largePad[channel]) Device::AnalogInput::SetUpDown(&largePad[channel], 1);
      if (ui != nullptr)
      {
        if (Device::AnalogInput::GetUDPtr() != &ui->fullScreen) Device::AnalogInput::SetUpDown(&ui->fullScreen, 1,-1);
        if (ui->fullScreen < 0) largePad[channel] = true;
        else largePad[channel] = false;
      }

      if(largePad[channel] && !Device::KeyPad::fnState.active())                                                         // full screen keyboard
      {
        Color chordColor = nodesInfo[NODE_CHORD].color;
        Color arpColor = nodesInfo[NODE_ARP].color;
        MatrixOS::LED::SetColor(Point(0, 0), Color(chordColor).DimIfNot(FindNode(NODE_CHORD)));
        MatrixOS::LED::SetColor(Point(0, 1), Color(arpColor).DimIfNot(FindNode(NODE_ARP)));
        multiPad->dimension = MultiPadDimension[FULL_PAD];
        multiPad->Render(origin + MultiPadOrigin[FULL_PAD]);
        return true;
      }

      for(uint8_t x = 0; x < dimension.x; x++)                                      // set all pixel to blank
        for(uint8_t y = 0; y < dimension.y; y++)
          MatrixOS::LED::SetColor(Point(x, y) + origin, Color(BLANK));
      // MatrixOS::LED::SetColor(origin, Color(RED));                                  // the back button

      if(ui != nullptr && ui->fullScreen && !Device::KeyPad::fnState.active())
      { 
        ui->Render(origin); 
        if(ui->enableMiniPad)
        {
            multiPad->dimension = MultiPadDimension[MINI_PAD];
            multiPad->Render(origin + MultiPadOrigin[MINI_PAD]);
        }
        return true; 
      }

      if (Device::KeyPad::fnState.active() || activeUI[channel] == NODE_NONE)       // node router
      {
        Color padColor;
        switch(channelConfig->padType[channel])
        {
          case NOTE_PAD:  padColor = COLOR_NOTE_PAD [1]; break;
          case PIANO_PAD: padColor = COLOR_PIANO_PAD[1]; break;
          case DRUM_PAD:  padColor = COLOR_DRUM_PAD [1]; break;
        }
        // MatrixOS::LED::SetColor(origin, Color(BLANK));
        // MatrixOS::LED::SetColor(origin + Point(0, 1), padColor);
        LargeButtonRender(NODE_SEQ, origin + Point(2, 0));
        AppButtonRender(NODE_CHORD, origin + Point(6, 0));
        AppButtonRender(NODE_ARP, origin + Point(8, 0));
        LargeButtonRender(NODE_FEEDBACK, origin + Point(10, 0));
      }
      else if (ui != nullptr)                                          
        ui->Render(origin);                                                         // half screen ui
      multiPad->dimension = MultiPadDimension[HALF_PAD];
      multiPad->Render(origin + MultiPadOrigin[HALF_PAD]);

      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) { 

      if (Device::KeyPad::fnState.active() || activeUI[channel] == NODE_NONE)
      {
        if (xy.y > 1)  return multiPad->KeyEvent(xy - MultiPadOrigin[HALF_PAD], keyInfo);
        switch(xy.x)
        {
          case 2: case 3: case 4: case 5:
            holdFN = false;
            return LargeButtonKeyEvent(NODE_SEQ,    xy, Point(2, 0), keyInfo);
          case 6: case 7:
            holdFN = false;
            return AppButtonKeyEvent(NODE_CHORD,  xy, Point(6, 0), keyInfo);
          case 8: case 9:
            holdFN = false;
            return AppButtonKeyEvent(NODE_ARP,    xy, Point(8, 0), keyInfo);
          case 10: case 11: case 12: case 13:
            holdFN = false;
            return LargeButtonKeyEvent(NODE_FEEDBACK, xy, Point(10, 0), keyInfo);
        }
      }
      else
      {
        if (largePad[channel])
        {
          if (multiPad->KeyEvent(xy - MultiPadOrigin[FULL_PAD], keyInfo)) return true; // full screen keyboard

          if (xy == Point(0, 0)) // chord
          {
            if(keyInfo->state == RELEASED && keyInfo->hold == false)
            {
              if (FindNode(NODE_CHORD)) NodeOff(NODE_CHORD);
              else NodeOn(NODE_CHORD);
              return true;
            }
          }
        
          if (xy == Point(0, 1)) // arp
          {
            if(keyInfo->state == RELEASED && keyInfo->hold == false)
            {
              if (FindNode(NODE_ARP)) NodeOff(NODE_ARP);
              else NodeOn(NODE_ARP);
              return true;
            }        
          }     
        }
        else
        {
          if (ui != nullptr && ui->fullScreen)
          {
            if(ui->enableMiniPad)
            {
                if(xy.x >= MultiPadOrigin[MINI_PAD].x && xy.x < MultiPadOrigin[MINI_PAD].x + MultiPadDimension[MINI_PAD].x &&
                   xy.y >= MultiPadOrigin[MINI_PAD].y && xy.y < MultiPadOrigin[MINI_PAD].y + MultiPadDimension[MINI_PAD].y)
                {
                    return multiPad->KeyEvent(xy - MultiPadOrigin[MINI_PAD], keyInfo);
                }
            }
            return ui->KeyEvent(xy, keyInfo);
          }

          if (xy.y > 1)  return multiPad->KeyEvent(xy - MultiPadOrigin[HALF_PAD], keyInfo);       // half screen keyboard
          else if (ui != nullptr) return ui->KeyEvent(xy, keyInfo);                               // half screen ui
        }
      }

      return false;
    }

  private:

    void AppButtonRender(NodeID nodeID, Point xy)
    {
      Color thisColor = nodesInfo[nodeID].color.DimIfNot(activeUI[channel] == nodeID);
      Color switchColor = Color(WHITE);

      MatrixOS::LED::SetColor(xy + Point(0, 0), thisColor);
      MatrixOS::LED::SetColor(xy + Point(1, 0), switchColor.DimIfNot(FindNode(nodeID)));
      MatrixOS::LED::SetColor(xy + Point(0, 1), thisColor);
      MatrixOS::LED::SetColor(xy + Point(1, 1), thisColor);
    }

    bool AppButtonKeyEvent(NodeID nodeID, Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if (ui == Point(1, 0))
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          if (FindNode(nodeID)) NodeOff(nodeID);
          else NodeOn(nodeID);
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          string on_off = FindNode(nodeID) ? " Off" : " On";
          MatrixOS::UIInterface::TextScroll(nodesInfo[nodeID].name + on_off, nodesInfo[nodeID].color);
        }
      }
      else
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          SetUI(nodeID);
          if(!FindNode(nodeID)) NodeOn(nodeID);
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll(nodesInfo[nodeID].name, nodesInfo[nodeID].color);
        }
      }
      return false;
    }

    void LargeButtonRender(NodeID nodeID, Point origin)
    {
      for(uint8_t x = 0; x < 4; x++) {
        for(uint8_t y = 0; y < 2; y++) {
          Point xy = origin + Point(x, y);
          MatrixOS::LED::SetColor(xy, nodesInfo[nodeID].color.DimIfNot(activeUI[channel] == nodeID));
        }
      }
    }

    bool LargeButtonKeyEvent(NodeID nodeID,Point xy, Point offset, KeyInfo* keyInfo)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        SetUI(nodeID);
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(nodesInfo[nodeID].name, nodesInfo[nodeID].color);
      }
      return false;
    }

    bool FindNode(NodeID nodeID)
    {
      return nodesInChannel[channel].find(nodeID) != nodesInChannel[channel].end();
    }
  };
}

