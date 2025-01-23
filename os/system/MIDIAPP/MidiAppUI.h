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

    enum MultiPadType                       {FULL_PAD,          HALF_PAD,           MINI_PAD};
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
      
      for(uint8_t x = 0; x < dimension.x; x++)                                      // set all pixel to blank
        for(uint8_t y = 0; y < dimension.y; y++)
          MatrixOS::LED::SetColor(Point(x, y) + origin, Color(BLANK));

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

  extern MidiAppUI* midiAppUI;
  extern SEQ_DataStore* seqData;

  class SubMidiAppUI: public UIComponent
  {
  public:
    Dimension dimension = Dimension(4, 1);

    std::unordered_set<uint16_t> actived;
    pair<uint16_t, uint8_t>* buttons = FBButtons2;
    uint8_t count = 8;
    bool shift = false;

    ~SubMidiAppUI() 
    {
      for (auto it = actived.begin(); it != actived.end(); it++)
      {
        MatrixOS::MidiCenter::Send_Off(ID_Type(*it), ID_Channel(*it), ID_Byte1(*it));
      }
      actived.clear();
    }

    virtual Dimension GetSize() { return dimension; }
    virtual Color GetColor() { return Color(WHITE); }

    virtual bool Render(Point origin) { 

      if (dimension.Area() < count && shift != Device::KeyPad::Shift())
      {
        for (auto it = actived.begin(); it != actived.end(); it++)
        {
          MatrixOS::MidiCenter::Send_Off(ID_Type(*it), ID_Channel(*it), ID_Byte1(*it));
        }
        actived.clear();
        shift = Device::KeyPad::Shift();
      }

      switch(midiAppUI->activeUI[midiAppUI->channel])
      {
        case NODE_SEQ:
          return SeqRender(origin);
        case NODE_ARP:
          return EmptyRender(origin);
        case NODE_CHORD:
          return EmptyRender(origin);
        case NODE_FEEDBACK:
          return FeedBackRender(origin);
        default:
          return EmptyRender(origin);
      }
      return true; }

      virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) 
      { 
        switch(midiAppUI->activeUI[midiAppUI->channel])
        {
          case NODE_SEQ:
            return SeqKeyEvent(xy, keyInfo);
          case NODE_ARP:
            return false;
          case NODE_CHORD:
            return false;
          case NODE_FEEDBACK:
            return FeedBackKeyEvent(xy, keyInfo);
          default:
            return false;
        }
        return false; 
      }

    private:
    bool EmptyRender(Point origin)
    {
      for(uint8_t x = 0; x < dimension.x; x++)
        for(uint8_t y = 0; y < dimension.y; y++)
          MatrixOS::LED::SetColor(origin + Point(x, y), Color(BLANK));
      return true;
    }

    bool SeqRender(Point origin)
    {
      Color copyColor = Color(CYAN);
      Color deleteColor = Color(RED);
      Color undoRedoColor = Color(shift ? GOLD : ORANGE);
      bool hasUndoRedo = shift ? seqData->CanRedo() : seqData->CanUndo();
      Color captureColor = Color(LAWN);
      bool hasCapture = seqData->HasCapture();

      MatrixOS::LED::SetColor(origin + Point(0, 0), copyColor.Blink_Color(seqData->editBlock.copyKeyHeld, Color(WHITE)));
      MatrixOS::LED::SetColor(origin + Point(1, 0), deleteColor.Blink_Color(seqData->editBlock.deleteKeyHeld, Color(WHITE)));
      MatrixOS::LED::SetColor(origin + Point(2, 0), undoRedoColor.DimIfNot(hasUndoRedo)); 
      MatrixOS::LED::SetColor(origin + Point(3, 0), captureColor.DimIfNot(hasCapture));
      return true;
    }

    bool SeqKeyEvent(Point xy, KeyInfo* keyInfo)
    {
      if(xy == Point(0, 0)) return CopyBtnKeyEvent(keyInfo);
      if(xy == Point(1, 0)) return DeleteBtnKeyEvent(keyInfo);

      if(keyInfo->state == RELEASED && keyInfo->hold == false)
      {
        if(xy == Point(2, 0))
        {
          if(shift) seqData->Redo();
          else seqData->Undo();
          return true;
        }
        if(xy == Point(3, 0))
        {
          if(!seqData->HasCapture()) return false;
          SEQ_Snapshot* cap = seqData->GetCapture();
          if(shift) seqData->ClearCapture();
          else 
          {
            seqData->CreateTempSnapshot(cap->position);
            seqData->EnableTempSnapshot();
            seqData->EnableCapture();
          }
          return true;
        }
        return false;
      }

      if(keyInfo->state == HOLD)
      {
        if(xy == Point(2, 0))
        {
          MatrixOS::UIInterface::TextScroll(shift ? "Redo" : "Undo", Color(shift ? GOLD : ORANGE));
          return true;
        }
        if(xy == Point(3, 0))
        {
          MatrixOS::UIInterface::TextScroll("Capture", Color(LAWN));
          return true;
        }
        return false;
      }
      return false;
    }

    bool DeleteBtnKeyEvent(KeyInfo* keyInfo)
    {
      if(keyInfo->state == PRESSED)
      {
        seqData->editBlock.deleteKeyHeld = true;
        seqData->editBlock.state = EDIT_NONE;
        seqData->editBlock.copyKeyHeld = false;
        return true;
      }
      else if(keyInfo->state == RELEASED)
      {
        if(!shift) seqData->editBlock = SEQ_EditBlock();
        return true;
      }
      return false;
    }

    bool CopyBtnKeyEvent(KeyInfo* keyInfo)
    {
        if(keyInfo->state == PRESSED)
        {
          seqData->editBlock.copyKeyHeld = true;
          seqData->editBlock.state = EDIT_NONE;
          seqData->editBlock.deleteKeyHeld = false;
          return true;
        }
        else if(keyInfo->state == RELEASED)
        {
          if(!shift)seqData->editBlock = SEQ_EditBlock();
          return true;
        }
        return false;
    }

    bool FeedBackRender(Point origin)
    {
      for (int x = 0; x < dimension.x; x++)
      {
        for (int y = 0; y < dimension.y; y++)
        {
          uint8_t n = x + y * dimension.x;
          MatrixOS::LED::SetColor(origin + Point(x, y), Color(palette[buttons[n + shift * count / 2].second]));
        }
      }
      return true;
    }

    bool FeedBackKeyEvent(Point xy, KeyInfo* keyInfo)
    {
      uint8_t n = xy.x + xy.y * dimension.x;
      
      uint16_t midiID = buttons[n + shift * count / 2].first;
      if (keyInfo->state == PRESSED){
        MatrixOS::MidiCenter::Send_On(ID_Type(midiID), ID_Channel(midiID), ID_Byte1(midiID), 127);
        actived.insert(midiID);
        return true;
      }

      if (keyInfo->state == RELEASED){
        MatrixOS::MidiCenter::Send_Off(ID_Type(midiID), ID_Channel(midiID), ID_Byte1(midiID));
        actived.erase(midiID);
        return true;
      }
      return false;
    }

  };


}

