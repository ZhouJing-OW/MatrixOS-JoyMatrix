#pragma once
#include "MatrixOS.h"
#include "MidiCenter.h"
#include "../os/ui/component/UIComponent.h"
#include "../os/ui/UIComponents.h"


namespace MatrixOS::MidiCenter
{
  class NodeUI : public UIComponent {
    public:
    Dimension dimension = Dimension(16, 2);
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
  };
}