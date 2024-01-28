#pragma once

#include "MatrixOS.h"
#include "MidiApp.h"
#include <math.h>
#include <vector>
#include <map>
#include <set>
#include "algorithm"

#define NODES_PER_CHANNEL 8

namespace MatrixOS::MidiCenter
{
  extern TransportState transportState;

  extern std::map<uint16_t, uint16_t> hold; // keyID , midiID
  extern std::set<uint16_t> toggle; // midiID
  extern std::set<uint16_t> midiIDList; // midiID, midiIDCount
  extern std::map<uint16_t, uint32_t> arp; // midiID, length

  extern uint8_t afterTouch;
  extern Timer beatTimer;
  
  extern double tickInterval;
  extern uint32_t playStartTime;
  extern uint32_t tickCount;
  
  void HoldNoteMoveToToggle();
  bool Scan_Hold();
  bool Scan_Toggle();
  bool Scan_Arp();
  void MidiRouter(RouterNode from, uint8_t type, uint8_t channel, uint8_t byte1, uint8_t byte2);
  void NodeInsert(RouterNode node, uint8_t channel, void *config);
  void NodeDelete(RouterNode node, uint8_t channel);
  void Send_On(int8_t type, int8_t channel, int8_t byte1, int8_t byte2);
  void Send_Off(int8_t type, int8_t channel, int8_t byte1);
  void RouterSetup(RouterNode* routerIndex, std::map<RouterNode, void*> nodeConfigs);
  void RouterClear();
} 