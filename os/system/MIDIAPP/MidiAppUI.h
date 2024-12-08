#pragma once
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
    bool largePad[16];
    bool holdFN = false;

    MidiAppUI(Dimension dimension = Dimension(16, 4))
    {
      this->dimension = dimension;
      memset(activeUI,  NODE_SEQ,   sizeof(activeUI));
      memset(lastUI,    NODE_NONE,  sizeof(lastUI));
      memset(largePad,  false,      sizeof(largePad));
      SetUI(NODE_SEQ);
    }

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

      for(uint8_t x = 0; x < dimension.x; x++)                                      // set all pixel to blank
        for(uint8_t y = 0; y < dimension.y; y++)
          MatrixOS::LED::SetColor(Point(x, y) + origin, Color(BLANK));
      MatrixOS::LED::SetColor(origin, Color(RED));                                  // the back button

      if(ui != nullptr && ui->fullScreen && !Device::KeyPad::fnState.active())      // full screen node
      { ui->Render(origin); return true; }

      if(largePad[channel] && activeUI[channel] == NODE_NONE)                       // full screen keyboard
      {
        multiPad->dimension = Dimension(16, 4);
        multiPad->Render(origin);
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
        MatrixOS::LED::SetColor(origin, Color(BLANK));
        MatrixOS::LED::SetColor(origin + Point(0, 1), padColor);
        SeqButtonRender(origin + Point(6, 0));
        AppButtonRender(NODE_ARP, origin + Point(12, 0));
        AppButtonRender(NODE_CHORD, origin + Point(10, 0));
      }
      else if (ui != nullptr)                                          
        ui->Render(origin);                                                         // half screen ui
      multiPad->dimension = Dimension(16, 2);                                       // half screen keyboard
      multiPad->Render(origin + Point(0, 2));

      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) { 
      if(ui != nullptr && ui->fullScreen && !Device::KeyPad::fnState.active())      // full screen Node
        return ui->KeyEvent(xy, keyInfo);
      
      if (xy == Point(0, 1))                                                        // the full screen keyboard button
      {
        if(Device::KeyPad::fnState.active() || activeUI[channel] == NODE_NONE)
        {
          if (keyInfo->state == RELEASED && keyInfo->hold == false)
          {
            if(activeUI[channel] != NODE_NONE) lastUI[channel] = activeUI[channel];
            SetUI(NODE_NONE);
            largePad[channel] = true;

            return true;
          }
          
          if (keyInfo->state == HOLD && largePad[channel] == false)
          {
            Color padColor;
            switch(channelConfig->padType[channel])
            {
              case NOTE_PAD: padColor = COLOR_NOTE_PAD[1]; break;
              case PIANO_PAD: padColor = COLOR_PIANO_PAD[1]; break;
              case DRUM_PAD: padColor = COLOR_DRUM_PAD[1]; break;
            }
            MatrixOS::UIInterface::TextScroll("large Pad", padColor);
            return true;
          }
        }
      }

      bool ret = false;
      if (largePad[channel])       ret = multiPad->KeyEvent(xy, keyInfo);           // full screen keyboard
      else if (xy.y > 1)  ret = multiPad->KeyEvent(xy - Point(0, 2), keyInfo);      // half screen keyboard
      else if (Device::KeyPad::fnState.active() || activeUI[channel] == NODE_NONE)  // node router
      {
        switch(xy.x)
        {
          case 6: case 7: case 8: case 9:
            holdFN = false;
            return SeqButtonKeyEvent(xy, Point(6, 0), keyInfo);
          case 10: case 11:
            holdFN = false;
            return AppButtonKeyEvent(NODE_CHORD,  xy, Point(10, 0), keyInfo);
          case 12: case 13:
            holdFN = false;
            return AppButtonKeyEvent(NODE_ARP,    xy, Point(12, 0), keyInfo);
        }
      }
      else if (ui != nullptr)
        ret =  ui->KeyEvent(xy, keyInfo);
      if(ret) return ret;
      
      if (xy == Point(0, 0))                                                        // the back button for all node
      {
        if (keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          if(largePad[channel]) 
          {
            SetUI(lastUI[channel]);
            largePad[channel] = false;
            lastUI[channel] = NODE_NONE;
          }
          else SetUI(NODE_NONE);
          return true;
        }
        
        if (keyInfo->state == HOLD && (largePad[channel] || activeUI[channel] != NODE_NONE))
        {
          MatrixOS::UIInterface::TextScroll("Back", Color(RED));
          return true;
        }
      }

      return false;
    }

  private:

    void AppButtonRender(NodeID nodeID, Point xy)
    {
      Color thisColor = nodesInfo[nodeID].color.ToLowBrightness(FindNode(nodeID));
      Color switchColor = Color(WHITE);

      MatrixOS::LED::SetColor(xy + Point(0, 0), thisColor);
      MatrixOS::LED::SetColor(xy + Point(1, 0), switchColor.ToLowBrightness(FindNode(nodeID)));
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

    void SeqButtonRender(Point origin)
    {
      for(uint8_t x = 0; x < 4; x++) {
        for(uint8_t y = 0; y < 2; y++) {
          Point xy = origin + Point(x, y);
          MatrixOS::LED::SetColor(xy, nodesInfo[NODE_SEQ].color);
        }
      }
    }

    bool SeqButtonKeyEvent(Point xy, Point offset, KeyInfo* keyInfo)
    {
      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        SetUI(NODE_SEQ);
        return true;
      }
      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(nodesInfo[NODE_SEQ].name, nodesInfo[NODE_SEQ].color);
      }
      return false;
    }

    bool FindNode(NodeID nodeID)
    {
      return nodesInChannel[channel].find(nodeID) != nodesInChannel[channel].end();
    }
  };
}

