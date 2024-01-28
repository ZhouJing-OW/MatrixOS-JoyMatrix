#include "MidiCenter.h"

namespace MatrixOS::MidiCenter
{
  std::set<RouterNode> routerNodes[16]; // 16 channel router node set
  
  void MidiRouter(RouterNode from, uint8_t type, uint8_t channel, uint8_t byte1, uint8_t byte2)
  {
    if (type != SEND_NOTE)
    {
      if(byte2 > 0)
        Send_On(type, channel, byte1, byte2);
      else
        Send_Off(type, channel, byte1);
      return;
    }

    afterTouch = byte2;
    uint8_t order = from >> 1;
    RouterNode to = NODE_MIDIOUT;
    for (auto it = routerNodes[channel].begin(); it != routerNodes[channel].end(); it++)
    {
      if((*it >> 1) > order)
      {
        to = *it;
        break;
      }
    }

    switch(to)
    {
      case NODE_CHORD:
        break;
      case NODE_ARP:
      {
        Arpeggiator* arp = arpeggiators[channel];
        if(byte2 == 0)
          arp->inputList.erase(byte1);
        else
        {
          uint8_t order = 0;
          for (auto it2 = arp->inputList.begin(); it2 != arp->inputList.end(); it2++)
          {
            order = it2->second > order ? it2->second : order;
          }
          order = order + 1;
          arp->inputList.insert({byte1, order});
          arp->inputVelocity = byte2;
        }
        // MLOGD("Midi Router", "Note Route into ARP channel: %d, note: %d, velocity: %d", channel, byte1, byte2);
        // MLOGD("Midi Router", "ARP inputList size: %d", arp->inputList.size());
        break;
      }
      case NODE_HUMANIZER:
        break;
      
      case NODE_MIDIOUT:
      default:
        if (byte2 == 0)
          MatrixOS::MIDI::Send(MidiPacket(0, NoteOff, channel, byte1, byte2));
        else
          MatrixOS::MIDI::Send(MidiPacket(0, NoteOn, channel, byte1, byte2));
        break;
    }
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
      afterTouch = byte2;
      break;
    }
  }

  void Send_Off(int8_t type, int8_t channel, int8_t byte1)
  {
    switch(type) {
    case SEND_NONE: break;
    case SEND_CC: 
      MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, channel, byte1, 0)); 
      break;
    case SEND_PC: break;
    case SEND_NOTE:
      MatrixOS::MIDI::Send(MidiPacket(0, NoteOff, channel, byte1, 0));
      break;
    }
  }

  void NodeInsert(RouterNode node, uint8_t channel, void *config)
  {
    if (channel > 15) return;

    routerNodes[channel].insert(node);
    switch(node)
    {
      case NODE_ARP:
        arpeggiators.insert({channel, new Arpeggiator((ArpConfig*)config, channel)});
        break;
      default:
        break;
    }
  }

  void NodeDelete(RouterNode node, uint8_t channel)
  {
    routerNodes[channel].erase(node);
    switch(node)
    {
      case NODE_ARP:
      {
        auto it = arpeggiators.find(channel);
        if(it != arpeggiators.end())
        {
          delete it->second;
          arpeggiators.erase(it);
        }
        break;
      }
      default:
        break;
    }
  }

  void RouterSetup(RouterNode* routerIndex, std::map<RouterNode, void*> nodeConfigs)
  {
    for(uint8_t i = 0; i < 16; i++)
    {
      for(uint8_t j = 0; j < NODES_PER_CHANNEL; j++)
      {
        RouterNode node = routerIndex[i * NODES_PER_CHANNEL + j];
        if(node != NODE_NONE)
        {
          NodeInsert(node, i, nodeConfigs[node]);
        }
      }
    }
  }

  void RouterClear()
  {
    for(uint8_t i = 0; i < 16; i++)
    {
      routerNodes[i].clear();
    }
    for(auto it = arpeggiators.begin(); it != arpeggiators.end(); it++)
    {
      delete it->second;
    }
    arpeggiators.clear();
  }

}

