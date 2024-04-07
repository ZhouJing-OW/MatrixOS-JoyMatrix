#pragma once

#include "MatrixOS.h"
#include "Node.h"
#include <math.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include "algorithm"

#define NODES_PER_CHANNEL 8
#define NODES_MAX_CONFIGS 16

enum PadCheck : uint8_t {
  IN_NONE, IN_INPUT, IN_ARP, IN_CHORD, IN_VOICE
};

namespace MatrixOS::MidiCenter
{
  extern TransportState     transportState;
  extern ChannelConfig*     channelConfig;
  extern NotePadConfig*     notePadConfig;
  extern MidiButtonConfig*  drumPadConfig;

  extern ArpConfig*         arpConfig;
  extern ChordConfig*       chordConfig;
  
  extern std::unordered_set<uint16_t>             CNTR_PadMidiID;   // midiID
  extern std::unordered_map<uint16_t, uint16_t>   CNTR_PadHold;     // keyID, midiID
  extern std::unordered_set<uint16_t>             CNTR_PadToggle;   // midiID
  extern std::unordered_map<uint16_t, uint32_t>   CNTR_Seq;         // midiID
  extern std::unordered_set<uint16_t>             CNTR_Chord;       // midiID
  extern std::unordered_map<uint16_t, uint32_t>   CNTR_Arp;         // midiID, length

  extern PadCheck padCheck[127]; // in globle channel;

  extern std::map<RouterNode, NodeInfo> nodesInfo;
  extern std::map<RouterNode, Node*>    nodesInChannel[16]; // 16 channel router node set
  extern std::map<RouterNode, uint8_t>  nodesConfigNum[16];

  extern uint8_t inputCount;
  extern bool timeReceived;
  extern Timer beatTimer;
  extern Timer stepTimer;
  extern double tickInterval;
  extern uint32_t playStartTime;
  extern uint32_t tickCount;

  void MoveHoldToToggle();
  
  bool Scan_Hold();
  bool Scan_Toggle();
  bool Scan_Arp();
  void PadColorCheck();
  PadCheck GetPadCheck(int8_t byte1);

  void NodeScan();

  void ClearHold(int8_t type = -1, int8_t channel = -1);
  void ClearToggle(int8_t type = -1, int8_t channel = -1);
  void ClearChord(int8_t channel = -1);
  void ClearArp(int8_t channel = -1);

  void MidiRouter(RouterNode from, uint8_t type, uint8_t channel, uint8_t byte1, uint8_t byte2);
  void Send_On(int8_t type, int8_t channel, int8_t byte1, int8_t byte2);
  void Send_Off(int8_t type, int8_t channel, int8_t byte1);
  void NodeInsert(uint8_t channel, RouterNode node, uint8_t configNum = 0);
  void NodeDelete(uint8_t channel, RouterNode node);
  bool RequestService(string name, ChannelConfig*& CH_Config);
  void EndService();
  void SetMidiAppNode(RouterNode node);

  uint16_t GetMidiID(uint8_t type, uint8_t channel, uint8_t byte1);
  uint8_t ID_Type(uint16_t midiID);
  uint8_t ID_Channel(uint16_t midiID);
  uint8_t ID_Byte1(uint16_t midiID);
  
} 