#include "MidiCenter.h"

#define midi_center_scanrate 1000

namespace MatrixOS::MidiCenter
{ 
  TransportState transportState;
  int16_t bpmPrv = 120;
  int16_t swingPrv = 50;
  int16_t tempDefVel = MatrixOS::UserVar::defaultVelocity;
  int16_t tempBright = std::sqrt((uint8_t)MatrixOS::UserVar::brightness);

  KnobConfig  bpm                 = {.lock = true, .data = {.varPtr = &bpmPrv},      .min = 20,  .max = 300,   .def = 120, .color = Color(PURPLE)};
  KnobConfig  swing               = {.lock = true, .data = {.varPtr = &swingPrv},    .max = 100, .def = 50,    .color = Color(GOLD)};
  KnobConfig  defaultVelocity     = {.lock = true, .data = {.varPtr = &tempDefVel},  .min = 1,   .def = 127,   .color = Color(LAWN)};
  KnobConfig  brightness          = {.lock = true, .data = {.varPtr = &tempBright},  .min = 2,   .max = 12,    .def = 4, .color = Color(WHITE)};

  std::vector<KnobConfig*> sysKnobs = {&bpm, &swing, &defaultVelocity, &brightness};
  
  bool timeReceived = false;
  Timer tickTimer;
  Timer beatTimer;
  Timer stepTimer;

  double tickInterval;
  uint32_t playStartTime;
  uint32_t tickCount;
  uint32_t halfTick;

  bool clockOut;
  bool clockIn;
  void MidiCenterTask(void* arg)
  {
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    struct MidiPacket midiPacket;
    
    uint32_t tickTime[4];
    for (;;)
    {
      while (MatrixOS::MIDI::Get(&midiPacket)) { 
        switch (midiPacket.status) {
          case Start:
          {
            if (!clockIn) break;
            timeReceived = true;
            tickCount = 0; halfTick = 0;
            tickTime[0] = MatrixOS::SYS::Millis();
            tickTimer.RecordCurrent();
            stepTimer.RecordCurrent();
            beatTimer.RecordCurrent();
            transportState.play = true;
            MLOGD("Midi Center", " clock started");
            break;
          }
          case Sync:
          {
            if (!clockIn) break;
            tickCount++; halfTick = tickCount * 2;
            tickTimer.RecordCurrent();
            if (tickCount % 8 == 0) {
              tickTime[tickCount % 24 / 8] = MatrixOS::SYS::Millis();
              stepTimer.RecordCurrent();
              if (tickCount % 24 == 0) {
                tickTime[3] = MatrixOS::SYS::Millis();
                uint32_t inputBPM = std::round(60000.0 / (middleOfThree(tickTime[1] - tickTime[0], tickTime[2] - tickTime[1], tickTime[3] - tickTime[2]) * 3));
                bpm.SetValue(inputBPM);
                tickInterval = (60000.0 / 24) / inputBPM;
                // MLOGD("Midi Center", "BPM: %d", inputBPM);
                beatTimer.RecordCurrent();
              }
            }
            break;
          }
          case Stop:
          {
            if (!clockIn) break;
            timeReceived = false;
            tickCount = 0; halfTick = 0;
            transportState.play = false;
            MLOGD("Midi Center", " clock stoped");
            break;
          }
          default:
            break;
        }
      }
      
      ClockOut();

      if(tickTimer.SinceLastTick() > tickInterval / 2 )
        halfTick = tickCount * 2 + 1;

      Scan_Toggle();
      Scan_Hold();
      Scan_Arp();

      uint32_t currentTime = xLastWakeTime;
      // if (currentTime != xLastWakeTime) MLOGD("Midi Center", "Task Time: %d, WakeTime: %d", currentTime, xLastWakeTime);
      uint8_t channelIndex = currentTime % 8 * 2;
      NodeScan(channelIndex); NodeScan(channelIndex + 1);
      
      if ((currentTime % (1000 / Device::fps)) == 0) { // scan by fps
        Panic();
        MoveHoldToToggle();
        PadColorCheck();

        clockOut = MatrixOS::UserVar::clockOut;
        clockIn  = MatrixOS::UserVar::clockIn;
        tickInterval = (60000.0 / 24) / bpm.Value();
        
        if(projectConfig != nullptr) // sync knobs to user var
        { 
          if(swingPrv != swing.Value()) {
            swingPrv = swing.Value();
            MatrixOS::FATFS::MarkChanged(projectConfig);
          }
          if(defaultVelocity.Value() != MatrixOS::UserVar::defaultVelocity) {
            MatrixOS::UserVar::defaultVelocity.Set(defaultVelocity.Value()); 
          }
          if(std::pow(brightness.Value(), 2) != MatrixOS::UserVar::brightness) {
            MatrixOS::UserVar::brightness.Set(std::pow(brightness.Value(), 2)); 
          }
        }
      }

      if((currentTime & 0x3FF) == 0) inputCount = 0;
      vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / midi_center_scanrate);
    }
  }

    void ClockOut()
  {
    if(!transportState.play) timeReceived = false;
    if(clockIn && timeReceived) return;

    if(bpmPrv != bpm.Value()) 
    {
      MatrixOS::FATFS::MarkChanged(projectConfig);
      tickInterval = (60000.0 / 24) / bpm.Value(); 
      bpmPrv = bpm.Value();
      tickCount = (MatrixOS::SYS::Millis() - playStartTime) / tickInterval;
      halfTick = tickCount * 2;
    }

    if(transportState.play && MatrixOS::SYS::Millis() >= playStartTime + (uint32_t)(tickInterval * tickCount)) {
      tickCount = (MatrixOS::SYS::Millis() - playStartTime) / tickInterval + 1;
      halfTick = tickCount * 2;
      if (clockOut) MatrixOS::MIDI::Send(MidiPacket(0, Sync));
      tickTimer.RecordCurrent();
      if(tickCount % 8 == 0)  {
        stepTimer.RecordCurrent(); 
        if(tickCount % 24 == 0) {
          beatTimer.RecordCurrent(); 
          // MLOGD("Midi Center", "BPM: %d Interval: %f", bpm.byte2, tickInterval);
        }
      }
    }
  }

  void ClockStart()
  {
    tickInterval = (60000.0 / 24) / bpm.Value();
    tickCount = 0;
    halfTick = 0;
    playStartTime = MatrixOS::SYS::Millis();
    transportState.play = true;
    MatrixOS::MIDI::Send(MidiPacket(0, Start));
    beatTimer.RecordCurrent();
  }

  void ClockStop()
  {
    tickCount = 0;
    transportState.play = false;
    MatrixOS::MIDI::Send(MidiPacket(0, Stop));
  }

  void Init()
  {
    xTaskCreate(MidiCenterTask, "MidiCenter", configMINIMAL_STACK_SIZE * 6, NULL, configMAX_PRIORITIES - 5, NULL);
  }

  bool Scan_Toggle()
  {    
    if (CNTR_PadToggle.empty()) return false;
    return false;
  }

  bool Scan_Hold()
  {
    if(CNTR_PadHold.empty()) return false;
    for (auto it = CNTR_PadHold.begin(); it != CNTR_PadHold.end();) 
    {
      KeyInfo* keyInfo = Device::KeyPad::GetKey(it->first);
      if(keyInfo->active() == false)
      {
        MidiRouter(NODE_KEYPAD, ID_Type(it->second), ID_Channel(it->second), ID_Byte1(it->second), 0);
        if (CNTR_PadHold.find(it->second) == CNTR_PadHold.end()) CNTR_PadMidiID.erase(it->second);
        it = CNTR_PadHold.erase(it);
      }
      else
        it++;
    }
    return !CNTR_PadHold.empty();
  }

  bool Scan_Arp()
  {
    for (auto it = CNTR_Arp.begin(); it != CNTR_Arp.end();)
    {
      if (MatrixOS::SYS::Millis() >= it->second)
      {
        MidiRouter(NODE_ARP, SEND_NOTE, ID_Channel(it->first), ID_Byte1(it->first), 0);
        it = CNTR_Arp.erase(it);
      }
      else it++;
    }
    return !CNTR_Arp.empty();
  }

  void PadColorCheck()
  {
    memset(padCheck, 0, sizeof(padCheck));
    uint8_t currentChannel = MatrixOS::UserVar::global_channel;

    if (!CNTR_SeqEditStep.empty())
    {
      SEQ_Step* step = CNTR_SeqEditStep.front().second;
      std::vector<SEQ_Note> notes = step->GetNotes();
      for (uint8_t i = 0; i < notes.size(); i++)
      {
        if(notes[i].number <= 127)
          padCheck[notes[i].number] = IN_SEQ;
      }
    }

    for(auto it : CNTR_PadMidiID) 
    {
      if (currentChannel == ID_Channel(it))
        padCheck[ID_Byte1(it)] = IN_INPUT;
    }

    for(auto it : CNTR_Chord)
    {
      if (currentChannel == ID_Channel(it))
      {
        uint8_t i = ID_Byte1(it) % 12;
        for (uint8_t octave = 0; octave < 10; octave++)
        {
          uint8_t note = i + octave * 12;
          if(i <= 127 && !padCheck[note])
            padCheck[note] = IN_VOICE;
        }
        padCheck[ID_Byte1(it)] = IN_CHORD;
      }
    }

    for(auto it : CNTR_Arp)
    {
      if (currentChannel == ID_Channel(it.first))
        padCheck[ID_Byte1(it.first)] = IN_ARP;
    }
  }

#define MIDIID_IS_FIRST     true
#define MIDIID_IS_SECOND    false

  template <typename TKey, typename TValue, bool IDpos = MIDIID_IS_FIRST>
  void ClearIDMap(std::map<TKey, TValue>& container, NodeID nodeID, bool eraseMidiID = true, int8_t type = -1, int8_t channel = -1) {
    if(container.empty()) return;

    for (auto it = container.begin(); it != container.end();) {
      uint16_t midiID = IDpos == MIDIID_IS_FIRST ? (uint16_t)it->first : (uint16_t)it->second;
      uint8_t ch = (midiID >> 8) & 0xF;
      if ((type == -1 || type == midiID >> 12) && (channel == -1 || ch == channel)) {
        uint8_t note = midiID & 0xFF;
        type = type == -1 ? midiID >> 12 : type;
        MidiRouter(nodeID, type, ch, note, 0);
        if (eraseMidiID) CNTR_PadMidiID.erase(midiID);
        it = container.erase(it);
      } else {
        it++;
      }
    }
  }

  void ClearIDSet(std::set<uint16_t>& container, NodeID nodeID, bool eraseMidiID = true, int8_t type = -1, int8_t channel = -1) {
    if (container.empty()) return;

    for (auto it = container.begin(); it != container.end();)
    {
      uint8_t ch = (*it >> 8) & 0xF;
      if ((type == -1 || type == (*it >> 12)) && (channel == -1 || ch == channel)) {
        uint8_t note = *it & 0xFF;
        type = type == -1 ? *it >> 12 : type;
        MidiRouter(nodeID, type, ch, note, 0);
        if (eraseMidiID) CNTR_PadMidiID.erase(*it);
        it = container.erase(it);
      } else {
        it++;
      }
    }
  }

  void ClearHold(int8_t type, int8_t channel)    
    { ClearIDMap<uint16_t, uint16_t, MIDIID_IS_SECOND>(CNTR_PadHold, NODE_KEYPAD, true, type, channel); }

  void ClearToggle(int8_t type, int8_t channel)  
    { ClearIDSet(CNTR_PadToggle, NODE_KEYPAD, true, type, channel); }

  void ClearChord(int8_t channel)   
    { ClearIDSet(CNTR_Chord, NODE_CHORD, false, -1, channel); }

  void ClearArp(int8_t channel)     
    { ClearIDMap<uint16_t, uint32_t, MIDIID_IS_FIRST>(CNTR_Arp, NODE_ARP, false, -1, channel); }

  void NodeScan(uint8_t channel){
    for (auto it = nodesInChannel[channel].begin(); it != nodesInChannel[channel].end(); it++) {
      it->second->Scan();
    }
  }
  
  ChannelConfig* GetChannelConfig() { return channelConfig; }
  TransportState* TransState() { return &transportState; }
  Timer* GetBeatTimer() { return &beatTimer; }

  std::vector<KnobConfig*> GetSysKnobs() {return sysKnobs;}

  uint16_t MidiID(uint8_t type, uint8_t channel, uint8_t byte1) { return (type << 12) | (channel << 8) | byte1; }
  uint8_t ID_Type(uint16_t midiID) { return midiID >> 12; }
  uint8_t ID_Channel(uint16_t midiID) { return (midiID >> 8) & 0xF; }
  uint8_t ID_Byte1(uint16_t midiID) { return midiID & 0xFF; }
  

}