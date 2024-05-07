#pragma once
#include "MatrixOS.h"
#include "MidiCenter.h"
#include "../os/ui/component/UIComponent.h"
#include "../os/ui/UIComponents.h"
#include <cstring>


namespace MatrixOS::MidiCenter
{
  class NodeUI : public UIComponent {
    public:
    Dimension dimension = Dimension(16, 2);
    bool fullScreen = false;
    uint8_t channel;
    uint8_t channelPrv;
    std::vector<KnobConfig*> knobPtr;
    NodeID currentNode = NODE_NONE;

    virtual void SetNull() = 0;
    virtual bool SetKnobPtr() = 0;
    
    virtual void On()
    {
      channel = MatrixOS::UserVar::global_channel;
      auto it = nodesConfigNum[channel].find(currentNode);
      uint8_t configNum = it != nodesConfigNum[channel].end() ? it->second : channel;
      MatrixOS::MidiCenter::NodeInsert(channel, currentNode, configNum);
      VarGet();
      if(SetKnobPtr()) {MatrixOS::KnobCenter::AddExtraPage(knobPtr); MatrixOS::KnobCenter::SetPage(4);}
      else MatrixOS::KnobCenter::DisableExtraPage();
    }

    virtual void Off()
    {
      channel = MatrixOS::UserVar::global_channel;
      MatrixOS::MidiCenter::NodeDelete(channel, currentNode);
      MatrixOS::KnobCenter::DisableExtraPage();
      SetNull();
    }

    virtual void VarGet(){}
    virtual void VarSet(){}
    
    Node* GetNodePtr(NodeID nodeID)
    {
      auto it = nodesInChannel[channel].find(nodeID);
      if (it != nodesInChannel[channel].end())
      {
        MLOGD("MidiNode", "%s in channel: %d Actived", nodesInfo[nodeID].name, channel + 1);
        Node* tempNode = it->second;
        return tempNode;
      }
      else
        return nullptr;
    }

    template <typename T>
    bool CheckNodeChange(T*& nodePtr, NodeID nodeID) 
    {
      channel = MatrixOS::UserVar::global_channel; // return true if channel changed
      if (channel != channelPrv || nodeID != currentNode)
      {
        channelPrv = channel;
        currentNode = nodeID;
        nodePtr = (T*)GetNodePtr(nodeID);   
        
        if (nodePtr == nullptr) { MatrixOS::KnobCenter::DisableExtraPage(); return false; }
        VarGet();
        if (SetKnobPtr()) {MatrixOS::KnobCenter::AddExtraPage(knobPtr); MatrixOS::KnobCenter::SetPage(4);}
        else MatrixOS::KnobCenter::DisableExtraPage();
        return true;
      } 
      
      if (nodePtr == nullptr) // return true if nodePtr was null but now it's not
      {
        nodePtr = (T*)GetNodePtr(nodeID);
        if (nodePtr == nullptr) return false;
        VarGet();
        if (SetKnobPtr()) {MatrixOS::KnobCenter::AddExtraPage(knobPtr); MatrixOS::KnobCenter::SetPage(4);}
        else MatrixOS::KnobCenter::DisableExtraPage();
        return true;
      }
      return false;
    }

    bool InArea(Point xy, Point offset, Dimension dimension) { return (xy.x >= offset.x) && (xy.x < offset.x + dimension.x) && (xy.y >= offset.y) && (xy.y < offset.y + dimension.y); }

    void ButtonRender(Point origin, Point offset, Color color) { MatrixOS::LED::SetColor(origin + offset, color); }
    
    #define BtnFunc std::function<void()>
    bool ButtonKeyEvent(Point xy, Point offset, KeyInfo* keyInfo, Color color, string name, BtnFunc callback)
    {
      if(keyInfo->state == RELEASED && !keyInfo->hold) callback();
      if(name != "" && keyInfo->state == HOLD) MatrixOS::UIInterface::TextScroll(name, color);
      return true;
    }

    template<size_t N, size_t M>
    void LabelRender(Dimension dimension, Point origin, Point offset, const char (&name)[N][M], const Color (&color)[N], uint8_t& data, bool whiteHighLight = false) 
    {
      for (uint8_t y = 0; y < dimension.y; y++) {
        for (uint8_t x = 0; x < dimension.x; x++) {
          Point xy = origin + offset + Point(x, y);
          uint8_t num = y * dimension.x + x;
          if(num >= N)
          {
            MatrixOS::LED::SetColor(xy, Color(BLANK));
            continue;
          }
          Color thisColor = std::strcmp(name[(uint8_t)data], name[num]) == 0 ? (whiteHighLight ? Color(WHITE) : color[num]) : (whiteHighLight ? color[num] : Color(color[num]).ToLowBrightness());
          MatrixOS::LED::SetColor(xy, thisColor);
        }
      }
    }

    template<size_t N, size_t M>
    bool LabelKeyEvent(Dimension dimension, Point xy, Point offset, KeyInfo* keyInfo, const char (&name)[N][M], const Color (&color)[N], uint8_t& data, uint8_t min, uint8_t max)
    {
      Point ui = xy - offset;
      uint8_t num = ui.y * dimension.x + ui.x;
      if (num >= N) return false;

      if(keyInfo->state == RELEASED && !keyInfo->hold)
      {
        data = num;
        return true;
      }

      if(keyInfo->state == HOLD)
      {
        MatrixOS::UIInterface::TextScroll(name[num], color[num]);
        return true;
      }
      return false;
    }
    
    template<typename T>
    void ValueBarRender(Dimension dimension, Point origin, Point offset, Color less, Color greater, T& data, uint8_t min, uint8_t max, Color zero = Color(BLANK))
    {
      float step = (max - min) / float(dimension.x * dimension.y - 1);
      for(uint8_t y = 0; y < dimension.y; y++) {
        for (uint8_t x = 0; x < dimension.x; x++) {
          Point xy = origin + offset + Point(x, y);
          uint8_t num = (y * dimension.x + x) * step + min;
          if(num == 0)
            MatrixOS::LED::SetColor(xy, zero);
          else
            MatrixOS::LED::SetColor(xy, num <= data ? less : greater);
        }
      }
    }

    template<typename T>
    bool ValueBarKeyEvent(Dimension dimension, Point xy, Point offset, KeyInfo* keyInfo, Color color, T& data, uint8_t min, uint8_t max)
    {
      Point ui = xy - offset;
      float step = (max - min) / float(dimension.x * dimension.y - 1);
      uint8_t num = (ui.y * dimension.x + ui.x) * step + min;
      num = num > max ? max : num;
      if(num < min || num > max) return false;

      if(keyInfo->state == RELEASED && !keyInfo->hold)
      {
        data = num;
        return true;
      }

      if(keyInfo->state == HOLD)
      {
        string value = std::to_string(num * 100 / max) + "%";
        MatrixOS::UIInterface::TextScroll(value, color);
        return true;
      }
      return false;
    }

  };

  

}