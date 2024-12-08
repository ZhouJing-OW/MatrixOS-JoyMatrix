#include "MidiCenter.h"

#define midi_center_scanrate 1000

namespace MatrixOS::MidiCenter
{ 
  TransportState transportState;
  int16_t bpmPrv = 120;
  int16_t swingPrv = 50;
  int16_t defVelPrv = MatrixOS::UserVar::defaultVelocity;
  int16_t brightnessPrv = std::sqrt((uint8_t)MatrixOS::UserVar::brightness);

  KnobConfig  bpm                 = {.lock = true, .data = {.varPtr = &bpmPrv},        .min = 20,  .max = 300,   .def = 120, .color = Color(PURPLE)};
  KnobConfig  swing               = {.lock = true, .data = {.varPtr = &swingPrv},      .max = 100, .def = 50,    .color = Color(GOLD)};
  KnobConfig  defVel              = {.lock = true, .data = {.varPtr = &defVelPrv},     .min = 1,   .def = 127,   .color = Color(LAWN)};
  KnobConfig  brightness          = {.lock = true, .data = {.varPtr = &brightnessPrv}, .min = 2,   .max = 12,    .def = 4, .color = Color(WHITE)};

  std::vector<KnobConfig*> sysKnobs = {&bpm, &swing, &defVel, &brightness};
  
  bool timeReceived = false;
  Timer tickTimer;
  Timer beatTimer;
  Timer stepTimer;

  double tickInterval;
  double quarterInterval;
  uint32_t taskCount;
  uint32_t tickCount;
  uint32_t quarterTick;

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
            tickCount = 0;
            quarterTick = 0;
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
            tickCount++;
            quarterTick = tickCount * 4;
            tickTimer.RecordCurrent();
            if (tickCount % 8 == 0) {
              tickTime[tickCount % 24 / 8] = MatrixOS::SYS::Millis();
              stepTimer.RecordCurrent();
              if (tickCount % 24 == 0) {
                tickTime[3] = MatrixOS::SYS::Millis();
                uint32_t inputBPM = std::round(60000.0 / (middleOfThree(tickTime[1] - tickTime[0], tickTime[2] - tickTime[1], tickTime[3] - tickTime[2]) * 3));
                bpm.SetValue(inputBPM);
                tickInterval = (60000.0 / 24) / inputBPM;
                quarterInterval = tickInterval / 4;
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
            tickCount = 0;
            quarterTick = 0;
            transportState.play = false;
            MLOGD("Midi Center", " clock stoped");
            break;
          }
          default:
            break;
        }
      }
      
      ClockOut();

      Scan_Toggle();
      Scan_Hold();
      Scan_Seq();
      Scan_Chord();
      Scan_Arp();

      if(transportState.play)
        quarterTick = tickCount * 4 + int(tickTimer.SinceLastTick() / quarterInterval);

      // if (currentTime != xLastWakeTime) MLOGD("Midi Center", "Task Time: %d, WakeTime: %d", currentTime, xLastWakeTime);
      
      for(uint8_t channelIndex = 0; channelIndex < 16; channelIndex++)
      {
        NodeScan(channelIndex);
      }
      
      if ((taskCount % (1000 / Device::fps)) == 0) { // scan by fps
        Panic();
        MoveHoldToToggle();
        PadColorCheck();

        clockOut = MatrixOS::UserVar::clockOut;
        clockIn  = MatrixOS::UserVar::clockIn;
        tickInterval = (60000.0 / 24) / bpm.Value();
        quarterInterval = tickInterval / 4;
        
        if(projectConfig != nullptr) // sync knobs to user var
        { 
          if(swingPrv != swing.Value()) {
            swingPrv = swing.Value();
            MatrixOS::FATFS::MarkChanged(projectConfig);
          }
          if(defVel.Value() != MatrixOS::UserVar::defaultVelocity) {
            MatrixOS::UserVar::defaultVelocity.Set(defVel.Value()); 
          }
          if(std::pow(brightness.Value(), 2) != MatrixOS::UserVar::brightness) {
            MatrixOS::UserVar::brightness.Set(std::pow(brightness.Value(), 2)); 
          }
        }
      }

      if((taskCount & 0x3FF) == 0) inputCount = 0;
      taskCount++;
      vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / midi_center_scanrate);
    }
  }

  double nextTickTime = 0;
  void ClockOut() {
    if(!transportState.play) 
    {
      timeReceived = false;
      if(nextTickTime != 0) ClockStop();
      return;
    }
    
    if(clockIn && timeReceived) return;

    if(bpmPrv != bpm.Value()) 
    {
      MatrixOS::FATFS::MarkChanged(projectConfig);
      tickInterval = (60000.0 / 24) / bpm.Value();
      quarterInterval = tickInterval / 4;
      bpmPrv = bpm.Value();
    }

    if(nextTickTime == 0) ClockStart();

    if(MatrixOS::SYS::Millis() >= nextTickTime) {
      tickCount++;
      quarterTick = tickCount * 4;
      nextTickTime += tickInterval;
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
    quarterInterval = tickInterval / 4;
    tickCount = 0;
    quarterTick = 0;
    transportState.play = true;
    if(clockOut)
    {
      MatrixOS::MIDI::Send(MidiPacket(0, Start));
      MatrixOS::MIDI::Send(MidiPacket(0, Sync));
    }
    nextTickTime = tickInterval + MatrixOS::SYS::Millis();
    tickTimer.RecordCurrent();
    stepTimer.RecordCurrent();
    beatTimer.RecordCurrent();
    MLOGD("Midi Center", "clock started");
  }

  void ClockStop()
  {
    tickCount = 0;
    quarterTick = 0;
    nextTickTime = 0;
    transportState.play = false;
    if(clockOut) MatrixOS::MIDI::Send(MidiPacket(0, Stop));
    MLOGD("Midi Center", "clock stoped");
  }

  void Init()
  {
    xTaskCreate(MidiCenterTask, "MidiCenter", configMINIMAL_STACK_SIZE * 6, NULL, configMAX_PRIORITIES - 5, NULL);
    taskCount = 0;
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

  bool Scan_Seq()
  {
    for (auto it = CNTR_Seq.begin(); it != CNTR_Seq.end();)
    {
      if (MatrixOS::SYS::Millis() >= it->second)
      {
        uint8_t channel = ID_Channel(it->first);
        MidiRouter(seqInOut[channel], SEND_NOTE, channel, ID_Byte1(it->first), 0);
        it = CNTR_Seq.erase(it);
      }
      else it++;
    }
    return !CNTR_Seq.empty();    
  }

  bool Scan_Chord()
  {
    for(auto it = CNTR_Chord.begin(); it != CNTR_Chord.end();)
    {
      uint8_t channel = ID_Channel(*it);
      if(nodesInChannel[channel].find(NODE_CHORD) == nodesInChannel[channel].end())
      {
        uint8_t note = ID_Byte1(*it);
        MidiRouter(NODE_CHORD, SEND_NOTE, channel, note, 0);
        it = CNTR_Chord.erase(it);
      }
      else it++;
    }
    return !CNTR_Chord.empty();
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

    if (!CNTR_SeqEditStep.empty())
    {
      SEQ_Step* step = CNTR_SeqEditStep.front().second;
      std::vector<const SEQ_Note*> notes = step->GetNotes();
      for (uint8_t i = 0; i < notes.size(); i++)
      {
        if(notes[i]->note <= 127)
          padCheck[notes[i]->note] = IN_SEQ;
      }
    }
    else
    {
      for(auto it : CNTR_Seq)
      {
        if (currentChannel == ID_Channel(it.first))
          padCheck[ID_Byte1(it.first)] = IN_SEQ;
      }
    }

    for(auto it : CNTR_PadMidiID) 
    {
      if (currentChannel == ID_Channel(it))
        padCheck[ID_Byte1(it)] = IN_INPUT;
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

  void ClearSeq(int8_t channel)     
    { ClearIDMap<uint16_t, uint32_t, MIDIID_IS_FIRST>(CNTR_Seq, NODE_SEQ, false, -1, channel); }

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