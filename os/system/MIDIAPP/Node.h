#pragma once
#include <map>
#include <queue>

enum RouterNode : uint8_t {
  NODE_NONE         = 0x00,
  NODE_ALL          = 0x00,
  
  NODE_INPUT        = 0x01,
  NODE_KEYPAD       = 0x02,
  NODE_SEQUENCER    = 0x03,
  NODE_CHORDSEQ     = 0x04,

  NODE_EUCLIDEAN    = 0x21,
  NODE_CHANCE777    = 0x22,
  NODE_M1SHA        = 0x23,

  NODE_SEQOUT1      = 0x51, // Just for sequencer input and output mark
  NODE_CHORD        = 0x60,

  NODE_SEQOUT2      = 0x71, // Just for sequencer input and output mark
  NODE_ARP          = 0x80,

  NODE_SEQOUT3      = 0x91, // Just for sequencer input and output mark
  NODE_MIDIFX       = 0xA0,

  NODE_MIDIOUT      = 0xFF,
};

namespace MatrixOS::MidiCenter
{
  void MidiRouter(RouterNode from, uint8_t type, uint8_t channel, uint8_t byte1, uint8_t byte2);

  struct NodeInfo {
    char name[16];
    Color color;
    void* configPtr;
  };

  struct NoteInfo {
    uint8_t velocity;
    uint32_t time;

    bool operator==(const NoteInfo& other) const {
      return velocity == other.velocity && time == other.time;
    }
  };

  class Node {
  public:
    RouterNode thisNode;
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
        inputList.insert({byte1, {byte2, MatrixOS::SYS::Millis()}});
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
#include "Chord.h"
