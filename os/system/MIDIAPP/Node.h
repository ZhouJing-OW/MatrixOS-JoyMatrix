#pragma once
#include <map>
#include <queue>

#define NODES_PER_CHANNEL 4
#define NODES_MAX_CONFIGS 16

enum NodeID : uint8_t {
  NODE_NONE         = 0x00,
  NODE_ALL          = 0x00,
  
  NODE_INPUT        = 0x01,
  NODE_KEYPAD       = 0x02,
  NODE_SEQ          = 0x03,
  NODE_CHORDSEQ     = 0x04,

  NODE_EUCLIDEAN    = 0x21,
  NODE_CHANCE777    = 0x22,
  NODE_M1SHA        = 0x23,

  BYPASS_SEQ1       = 0x51, // Just for sequencer input and output mark
  NODE_CHORD        = 0x60,

  BYPASS_SEQ2       = 0x71, // Just for sequencer input and output mark
  NODE_ARP          = 0x80,

  BYPASS_SEQ3       = 0x91, // Just for sequencer input and output mark
  NODE_MIDIFX       = 0xA0,

  NODE_MIDIOUT      = 0xFF,
};

namespace MatrixOS::MidiCenter
{
  void MidiRouter(NodeID from, uint8_t type, uint8_t channel, uint8_t byte1, uint8_t byte2);

  struct NodeInfo {
    char name[16];
    Color color;
    void* configPtr;
  };

  struct NoteInfo {
    int8_t velocity;
    uint32_t time;

    NoteInfo(uint8_t velocity, uint32_t time)
    {
      this->velocity = velocity;
      this->time = time;
    }

    bool operator==(const NoteInfo& other) const {
      return velocity == other.velocity && time == other.time;
    }
  };

  class Node {
  public:
    NodeID thisNode;
    uint8_t channel;
    uint8_t configNum;

    std::map<uint8_t, NoteInfo> inputList; // note , velocity,  time
    std::map<uint8_t, NoteInfo> inputListPrv; // note, time
    virtual void Scan(){};
    virtual ~Node() = default;

    void RouteIn(uint8_t byte1, uint8_t byte2)
    {
      if(byte2 == 0) {
        auto it = inputList.find(byte1);
        if (it != inputList.end())
          inputList.erase(byte1);
        else
          OutListNoteOff(byte1);
      }
      else
        inputList.insert_or_assign(byte1, NoteInfo(byte2, MatrixOS::SYS::Millis()));
    // MLOGD("Midi Router", "Note Route into ARP channel: %d, note: %d, velocity: %d", channel, byte1, byte2);
    // MLOGD("Midi Router", "ARP inputList size: %d", arp->inputList.size());
    }
    
    virtual void OutListNoteOff(uint8_t note)
    {
      MidiRouter(thisNode, SEND_NOTE, channel, note, 0);
    }
  };

}


#include "Arpeggiator.h"
#include "Chorder.h"
