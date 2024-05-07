#pragma once

#include "MatrixOS.h"
#include "Node.h"
#include "SeqData.h"
#include <math.h>
#include <cmath>
#include <vector>
#include <map>
#include <set>
#include "algorithm"

enum PadCheck : uint8_t {
  IN_NONE, IN_INPUT, IN_SEQ, IN_ARP, IN_CHORD, IN_VOICE
};

namespace MatrixOS::MidiCenter
{
  extern int16_t bpmPrv, swingPrv, defaultVelocityPrv, brightnessPrv;
  extern KnobConfig bpm, swing, defaultVelocity, brightness;

  extern TransportState     transportState;
  extern ProjectConfig*     projectConfig;
  extern ChannelConfig*     channelConfig;
  extern NotePadConfig*     notePadConfig;
  extern MidiButtonConfig*  drumPadConfig;

  extern ArpConfig*         arpConfig;
  extern ChorderConfig*     chordConfig;
  
  extern std::set<uint16_t>             CNTR_PadMidiID;   // midiID
  extern std::map<uint16_t, uint16_t>   CNTR_PadHold;     // keyID, midiID
  extern std::set<uint16_t>             CNTR_PadToggle;   // midiID
  extern std::map<uint16_t, uint32_t>   CNTR_Seq;         // midiID, offTime
  extern std::vector<std::pair<SEQ_Pos, SEQ_Step*>>   CNTR_SeqEditStep;
  extern std::set<uint16_t>             CNTR_Chord;       // midiID
  extern std::map<uint16_t, uint32_t>   CNTR_Arp;         // midiID, offTime

  extern PadCheck padCheck[127]; // in globle channel;

  extern std::map<NodeID, NodeInfo> nodesInfo;
  extern std::map<NodeID, Node*>    nodesInChannel[16]; // 16 channel router node set
  extern std::map<NodeID, uint8_t>  nodesConfigNum[16];
  extern NodeID seqInOut[16];

  extern uint8_t inputCount;
  extern bool timeReceived;

  extern Timer tickTimer;
  extern Timer stepTimer;
  extern Timer beatTimer;
  
  extern double tickInterval;
  extern uint32_t tickCount;
  extern uint32_t halfTick;

  void MoveHoldToToggle();
  
  bool Scan_Hold();
  bool Scan_Toggle();
  bool Scan_Seq();
  bool Scan_Chord();
  bool Scan_Arp();
  void PadColorCheck();
  PadCheck GetPadCheck(int8_t byte1);

  void NodeScan(uint8_t channel);

  void ClearHold(int8_t type = -1, int8_t channel = -1);
  void ClearToggle(int8_t type = -1, int8_t channel = -1);
  void ClearSeq(int8_t channel = -1);
  void ClearChord(int8_t channel = -1);
  void ClearArp(int8_t channel = -1);

  void MidiRouter(NodeID from, uint8_t type, uint8_t channel, uint8_t byte1, uint8_t byte2);
  void Send_On(int8_t type, int8_t channel, int8_t byte1, int8_t byte2);
  void Send_Off(int8_t type, int8_t channel, int8_t byte1);
  void NodeInsert(uint8_t channel, NodeID nodeID, uint8_t configNum = 0);
  void NodeDelete(uint8_t channel, NodeID nodeID);

  bool RequestService(string name, ChannelConfig*& CH_Config);
  void EndService();
  void SetMidiAppNode(NodeID nodeID);

  uint16_t MidiID(uint8_t type, uint8_t channel, uint8_t byte1);
  uint8_t ID_Type(uint16_t midiID);
  uint8_t ID_Channel(uint16_t midiID);
  uint8_t ID_Byte1(uint16_t midiID);
  
} 