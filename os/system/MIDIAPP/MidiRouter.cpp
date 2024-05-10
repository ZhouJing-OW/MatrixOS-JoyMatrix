#include "MidiCenter.h"
#include "MidiAppUI.h"

namespace MatrixOS::MidiCenter
{
  std::map<NodeID, Node*> nodesInChannel[16]; // 16 channel router node set
  std::map<NodeID, uint8_t> nodesConfigNum[16]; // temp config number for each node
  NodeID seqInOut[16] = {BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1,
                         BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1, BYPASS_SEQ1};

  ChorderConfig* chordConfig;
  ArpConfig* arpConfig;

  std::map<NodeID, NodeInfo> nodesInfo = {
    {NODE_NONE,   {"",        Color(BLANK),    nullptr     }},
    {NODE_SEQ,    {"SEQ",     Color(LAWN),     nullptr     }},
    {NODE_CHORD,  {"CHORD",   Color(YELLOW),   chordConfig }},
    {NODE_ARP,    {"ARP",     Color(ORANGE),   arpConfig   }},
  };

  void MidiRouter(NodeID from, uint8_t type, uint8_t channel, uint8_t byte1, uint8_t byte2)
  {
    if (type != SEND_NOTE)
    {
      if (byte2 != 0) Send_On(type, channel, byte1, byte2);
      else Send_Off(type, channel, byte1);
      return;
    }

    uint8_t order = (uint8_t)from | 0x0F;
    NodeID to = NODE_MIDIOUT;
    
    auto it = nodesInChannel[channel].lower_bound((NodeID)order);
    if (it != nodesInChannel[channel].end())
     to = (NodeID)it->first;
    
    if(seqData && seqInOut[channel] > (from | 0x0F) && seqInOut[channel] < to)
    {
      if(seqData->Capture(channel, byte1, byte2) == BLOCK) return;
    }

    if (to == NODE_MIDIOUT)
      MatrixOS::MIDI::Send(MidiPacket(0, byte2 == 0 ? NoteOff : NoteOn, channel, byte1, byte2));
    else
      nodesInChannel[channel][to]->RouteIn(byte1, byte2);

    
  }

  void Send_On(int8_t type, int8_t channel, int8_t byte1, int8_t byte2)
  {
    switch(type) {
    case SEND_NONE: break; // None
    case SEND_CC: // CC
      MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, channel, byte1, byte2)); 
      break;
    case SEND_PC: // PC
      MatrixOS::MIDI::Send(MidiPacket(0, ProgramChange, channel, byte2, byte2)); 
      break;
    case SEND_NOTE: // Note
      byte2 = Device::KeyPad::GetVelocity();
      MatrixOS::MIDI::Send(MidiPacket(0, NoteOn, channel, byte1, byte2)); 
      break;
    default: break;
    }
  }

  void Send_Off(int8_t type, int8_t channel, int8_t byte1)
  {
    switch(type) {
    case SEND_CC: 
      MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, channel, byte1, 0)); 
      break;
    case SEND_NOTE:
      MatrixOS::MIDI::Send(MidiPacket(0, NoteOff, channel, byte1, 0));
      break;
    default: break;
    }
  }

  void NodeInsert(uint8_t channel, NodeID nodeID, int8_t configNum)
  {
    if (channel > 15) return;
    if (configNum >= NODES_MAX_CONFIGS) configNum = NODES_MAX_CONFIGS - 1;
    if (nodesInChannel[channel].find(nodeID) != nodesInChannel[channel].end()) return;
    if (configNum == -1)
    {
      auto it = nodesConfigNum[channel].find(nodeID);
      configNum = it != nodesConfigNum[channel].end() ? it->second : channel;
      nodesConfigNum[channel].emplace(nodeID, configNum);
    }

    switch(nodeID)
    {
      case NODE_SEQ:
        nodesInChannel[channel].insert({NODE_SEQ,   new Sequencer(channel)});
        break;
      case NODE_CHORD:
        nodesInChannel[channel].insert({NODE_CHORD, new Chorder(channel, chordConfig, configNum)});
        break;
      case NODE_ARP:
        nodesInChannel[channel].insert({NODE_ARP,   new Arpeggiator(channel, arpConfig, configNum)});
        break;
      default:
        break;
    }
  }

  void NodeDelete(uint8_t channel, NodeID nodeID)
  {
    auto it = nodesInChannel[channel].find(nodeID);
    if(it != nodesInChannel[channel].end()) {
      delete it->second;
      nodesInChannel[channel].erase(it);
    }
  }

}

