#pragma once
#include "ArpeggiatorUI.h"
#include "ChorderUI.h"
#include "SequencerUI.h"
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
    NodeID activeUI;
    NodeID lastUI = NODE_NONE;
    bool largePad = false;
    bool holdFN = false;

    MidiAppUI(Dimension dimension = Dimension(16, 4))
    {
      this->dimension = dimension;
    }

    virtual Dimension GetSize() { return dimension; }
    UIComponent* GetUI() { return ui.get(); }

    void SetNode(NodeID nodeID = NODE_NONE) 
    {
      lastUI = nodeID == NODE_NONE ? activeUI : NODE_NONE;
      largePad = false;
      activeUI = nodeID;
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
      if(ui == nullptr) MatrixOS::KnobCenter::DisableExtraPage();
      else ui->VarGet();
    }

    inline void NodeOn(NodeID nodeID) { 
      if(activeUI == nodeID) ui->On();
      else NodeInsert(channel, nodeID);
    }

    inline void NodeOff(NodeID nodeID) { 
      if(activeUI == nodeID) ui->Off();
      else NodeDelete(channel, nodeID);
    }

    virtual bool Render(Point origin) {
      channel = MatrixOS::UserVar::global_channel;
      
      if (Device::KeyPad::fnState.active() || activeUI == NODE_NONE)
      {
        for(uint8_t x = 0; x < dimension.x; x++)
          for(uint8_t y = 0; y < dimension.y; y++)
            MatrixOS::LED::SetColor(Point(x, y) + origin, COLOR_BLANK);
        Color padColor;
        switch(channelConfig->padType[channel])
        {
          case NOTE_PAD: padColor = COLOR_NOTE_PAD[1]; break;
          case PIANO_PAD: padColor = COLOR_PIANO_PAD[1]; break;
          case DRUM_PAD: padColor = COLOR_DRUM_PAD[1]; break;
        }
        MatrixOS::LED::SetColor(origin + Point(0, 1), padColor);
        SeqButtonRender(origin + Point(6, 0));
        AppButtonRender(NODE_ARP, origin + Point(12, 0));
        AppButtonRender(NODE_CHORD, origin + Point(10, 0));
        
      }
      else if (ui != nullptr || !largePad)
      {
        for(uint8_t x = 0; x < 16; x++)
        {
          MatrixOS::LED::SetColor(origin + Point(x, 0), COLOR_BLANK);
          MatrixOS::LED::SetColor(origin + Point(x, 1), COLOR_BLANK);
        }
        MatrixOS::LED::SetColor(origin + Point(0, 0), COLOR_RED);
        ui->Render(origin);
      }

      
      if (Device::KeyPad::fnState.active() && largePad)
      { largePad = false; holdFN = true; } 
      if (!Device::KeyPad::fnState.active() && holdFN)
      { largePad = true; holdFN = false; }

      if(largePad) 
      {
        multiPad->dimension = Dimension(16, 4);
        MatrixOS::LED::SetColor(origin + Point(0, 0), COLOR_RED);
        MatrixOS::LED::SetColor(origin + Point(0, 1), COLOR_BLANK);
      }
      else multiPad->dimension = Dimension(16, 2);
      multiPad->Render(origin + (largePad ? Point(0,0) : Point(0, 2)));

      
      return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) { 
      if (xy == Point(0, 0))
      {
        if (keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          if(largePad) largePad = false;
          else SetNode(NODE_NONE);
          return true;
        }
        
        if (keyInfo->state == HOLD && (largePad || activeUI != NODE_NONE))
        {
          MatrixOS::UIInterface::TextScroll("Back", COLOR_RED);
          return true;
        }
      }

      if (xy == Point(0, 1) && activeUI == NODE_NONE)
      {
        if (keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          largePad = true;
          return true;
        }
        
        if (keyInfo->state == HOLD && largePad == false)
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

      if (largePad) return multiPad->KeyEvent(xy, keyInfo);
      if (xy.y > 1) return multiPad->KeyEvent(xy - Point(0, 2), keyInfo);

      if (Device::KeyPad::fnState.active() || activeUI == NODE_NONE)
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
        return ui->KeyEvent(xy, keyInfo);
      return false;
    }

  private:

    void AppButtonRender(NodeID nodeID, Point xy)
    {
      Color thisColor = nodesInfo[nodeID].color.ToLowBrightness(FindNode(nodeID));
      Color switchColor = COLOR_WHITE;

      MatrixOS::LED::SetColor(xy + Point(0, 0), thisColor);
      MatrixOS::LED::SetColor(xy + Point(1, 0), thisColor);
      MatrixOS::LED::SetColor(xy + Point(0, 1), thisColor);
      MatrixOS::LED::SetColor(xy + Point(1, 1), switchColor.ToLowBrightness(FindNode(nodeID)));
    }

    bool AppButtonKeyEvent(NodeID nodeID, Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if (ui == Point(1, 1))
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
          SetNode(nodeID);
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
        SetNode(NODE_SEQ);
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

