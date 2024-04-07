#pragma once
#include "ArpeggiatorUI.h"
#include "ChordUI.h"
#include "SequncerUI.h"
#include "MidiCenter.h"
#include <variant>
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
    RouterNode activeUI;
    RouterNode lastUI = NODE_NONE;
    bool largePad = false;
    bool holdFN = false;

    MidiAppUI(Dimension dimension = Dimension(16, 4))
    {
      this->dimension = dimension;
    }

    virtual Dimension GetSize() { return dimension; }
    UIComponent* GetUI() { return ui.get(); }

    void SetNode(RouterNode node = NODE_NONE) 
    {
      lastUI = node == NODE_NONE ? activeUI : NODE_NONE;
      largePad = false;
      activeUI = node;
      switch (node)
      {
        case NODE_ARP:
          ui = std::make_unique<ArpeggiatorUI>(); 
          break;
        case NODE_CHORD:
          ui = std::make_unique<ChordUI>(); 
          break;
        default:
          ui = nullptr;
          break;
      }
      if(ui == nullptr) MatrixOS::KnobCenter::DisableExtraPage();
      else ui->VarGet();
    }

    inline void NodeOn(RouterNode node) { 
      if(activeUI == node) ui->On();
      else NodeInsert(channel, node);
    }

    inline void NodeOff(RouterNode node) { 
      if(activeUI == node) ui->Off();
      else NodeDelete(channel, node);
    }

    virtual bool Render(Point origin) {
      channel = MatrixOS::UserVar::global_channel;
      
      if (Device::KeyPad::fnState.active() || activeUI == NODE_NONE)
      {
        for(uint8_t x = 0; x < dimension.x; x++)
          for(uint8_t y = 0; y < dimension.y; y++)
            MatrixOS::LED::SetColor(Point(x, y) + origin, COLOR_BLANK);
        Color PadColor;
        switch(channelConfig->padType[channel])
        {
          case NOTE_PAD: PadColor = COLOR_NOTE_PAD[1]; break;
          case PIANO_PAD: PadColor = COLOR_PIANO_PAD[1]; break;
          case DRUM_PAD: PadColor = COLOR_DRUM_PAD[1]; break;
        }
        MatrixOS::LED::SetColor(origin + Point(0, 1), PadColor);
        AppRender(NODE_ARP, origin + Point(12, 0));
        AppRender(NODE_CHORD, origin + Point(10, 0));
        
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
        
        if (keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("Back", lastUI == NODE_NONE ? COLOR_RED : nodesInfo[lastUI].color);
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
        
        if (keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll("large Pad", lastUI == NODE_NONE ? COLOR_RED : nodesInfo[lastUI].color);
          return true;
        }
      }


      if (largePad) return multiPad->KeyEvent(xy, keyInfo);
      if (xy.y > 1) return multiPad->KeyEvent(xy - Point(0, 2), keyInfo);

      if (Device::KeyPad::fnState.active() || activeUI == NODE_NONE)
      {
        switch(xy.x)
        {
          case 10: case 11:
            holdFN = false;
            return AppKeyEvent(NODE_CHORD,  xy, Point(10, 0), keyInfo);
          case 12: case 13:
            holdFN = false;
            return AppKeyEvent(NODE_ARP,    xy, Point(12, 0), keyInfo);
        }
      }
      else if (ui != nullptr)
        return ui->KeyEvent(xy, keyInfo);
      return false;
    }

  private:

    void AppRender(RouterNode node, Point xy)
    {
      Color thisColor = nodesInfo[node].color.ToLowBrightness(FindNode(node));
      Color switchColor = COLOR_WHITE;

      MatrixOS::LED::SetColor(xy + Point(0, 0), thisColor);
      MatrixOS::LED::SetColor(xy + Point(1, 0), thisColor);
      MatrixOS::LED::SetColor(xy + Point(0, 1), thisColor);
      MatrixOS::LED::SetColor(xy + Point(1, 1), switchColor.ToLowBrightness(FindNode(node)));
    }

    bool AppKeyEvent(RouterNode node, Point xy, Point offset, KeyInfo* keyInfo)
    {
      Point ui = xy - offset;
      if (ui == Point(1, 1))
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          if (FindNode(node)) NodeOff(node);
          else NodeOn(node);
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          string on_off = FindNode(node) ? " Off" : " On";
          MatrixOS::UIInterface::TextScroll(nodesInfo[node].name + on_off, nodesInfo[node].color);
        }
      }
      else
      {
        if(keyInfo->state == RELEASED && keyInfo->hold == false)
        {
          SetNode(node);
          return true;
        }
        if(keyInfo->state == HOLD)
        {
          MatrixOS::UIInterface::TextScroll(nodesInfo[node].name, nodesInfo[node].color);
        }
      }
      return false;
    }

    bool FindNode(RouterNode node)
    {
      return nodesInChannel[channel].find(node) != nodesInChannel[channel].end();
    }
  };
}

