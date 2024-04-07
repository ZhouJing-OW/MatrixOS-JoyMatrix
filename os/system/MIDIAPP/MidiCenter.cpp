#include "MidiCenter.h"

#define midi_center_scanrate 1000

namespace MatrixOS::MidiCenter
{
  TransportState transportState;
  KnobConfig bpm                = {.lock = true, .byte2 = MatrixOS::UserVar::BPM, .min = 20, .max = 300, .def = 120, .color = COLOR_PURPLE};
  KnobConfig swing              = {.lock = true, .byte2 = MatrixOS::UserVar::swing, .max = 100, .def = 50, .color = COLOR_GOLD};
  KnobConfig defaultVelocity    = {.lock = true, .byte2 = MatrixOS::UserVar::defaultVelocity, .min = 1, .def = 127, .color = COLOR_GREEN};
  KnobConfig brightness         = {.lock = true, .byte2 = MatrixOS::UserVar::brightness, .min = 4, .max = 100, .def = 16, .color = COLOR_WHITE};
  int16_t bpmPrv;
  int16_t swingPrv;
  int16_t defaultVelocityPrv;
  int16_t brightnessPrv;
  std::vector<KnobConfig*> sysKnobs = {&bpm, &swing, &defaultVelocity, &brightness};
  
  bool timeReceived = false;
  Timer beatTimer;
  Timer stepTimer;

  double tickInterval;
  uint32_t playStartTime;
  uint32_t tickCount;

  bool clockOut;
  bool clockIn;
  void MidiCenterTask(void* arg)
  {
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    struct MidiPacket midiPacket;
    
    uint16_t taskCount = 0;
    uint32_t tickTime[4];
    for (;;)
    {
      while (MatrixOS::MIDI::Get(&midiPacket)) { 
        switch (midiPacket.status) {
          case Start:
            if (!clockIn) break;
            timeReceived = true;
            tickCount = 0;
            tickTime[0] = MatrixOS::SYS::Millis();
            stepTimer.RecordCurrent();
            beatTimer.RecordCurrent();
            MLOGD("Midi Center", " clock started");
            break;
          case Sync:
            if (!clockIn) break;
            tickCount++;
            if (tickCount % 8 == 0) {
              tickTime[tickCount / 8] = MatrixOS::SYS::Millis();
              stepTimer.RecordCurrent();
              if (tickCount % 24 == 0) {
                tickCount = 0;
                tickTime[3] = MatrixOS::SYS::Millis();
                uint32_t inputBPM = std::round(60000.0 / (middleOfThree(tickTime[1] - tickTime[0], tickTime[2] - tickTime[1], tickTime[3] - tickTime[2]) * 3));
                bpm.byte2 = inputBPM;
                // MLOGD("Midi Center", "BPM: %d", inputBPM);
                beatTimer.RecordCurrent();
              }
            }
            break;
          case Stop:
            if (!clockIn) break;
            timeReceived = false;
            tickCount = 0;
            MLOGD("Midi Center", " clock stoped");
            break;
          default:
            break;
        }
      }
      
      ClockOut();

      #define MIDI_TASEK_SCANRATE (1000 + Device::fps / 2) / Device::fps
      
      if (taskCount % MIDI_TASEK_SCANRATE == 0) {// scan rate 100Hz
        Panic();
        MoveHoldToToggle();
        Scan_Toggle();
        Scan_Hold();
        Scan_Arp();
        PadColorCheck();

        NodeScan();
        clockOut = MatrixOS::UserVar::clockOut;
        clockIn = MatrixOS::UserVar::clockIn;

        if(swingPrv != swing.byte2) 
          {MatrixOS::UserVar::swing.Set(swing.byte2); swingPrv = swing.byte2;}
        if(defaultVelocityPrv != defaultVelocity.byte2) 
          {MatrixOS::UserVar::defaultVelocity.Set(defaultVelocity.byte2); defaultVelocityPrv = defaultVelocity.byte2;}
        if(brightnessPrv != brightness.byte2) 
          {MatrixOS::UserVar::brightness.Set(brightness.byte2); brightnessPrv = brightness.byte2;}
        if (taskCount % 100 == 0 && MatrixOS::UserVar::BPM != bpm.byte2)
          MatrixOS::UserVar::BPM.Set(bpm.byte2);
      }

      if ((taskCount % (1000 / Device::fps)) == 0) { // scan by fps
        
      }

      
      taskCount++;
      if(taskCount >= 1000)
      {
        taskCount = 0;
        inputCount = 0;
      }

      vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / midi_center_scanrate);
    }
  }

  void Init()
  {
    xTaskCreate(MidiCenterTask, "MidiCenter", configMINIMAL_STACK_SIZE * 4, NULL, configMAX_PRIORITIES - 5, NULL);
  }

  bool Scan_Toggle()
  {    
    if (CNTR_PadToggle.empty()) return false;
    return false;
  }

  bool Scan_Hold()
  {
    if(CNTR_PadHold.empty()) return false;
    for (auto it = CNTR_PadHold.begin(); it != CNTR_PadHold.end(); it++) 
    {
      uint16_t midiID = it->second;
      uint8_t type = ID_Type(midiID);
      uint8_t channel = ID_Channel(midiID);
      uint8_t byte1 = ID_Byte1(midiID);
      KeyInfo* keyInfo = Device::KeyPad::GetKey(it->first);
      
      if(keyInfo->state == RELEASED || keyInfo->state == CLEARED || keyInfo->state == IDLE)
      {
        MidiRouter(NODE_KEYPAD, type, channel, byte1, 0);
        CNTR_PadHold.erase(it);
        auto it_2 = CNTR_PadHold.begin();
        for( ; it_2 != CNTR_PadHold.end(); it_2++)
        {
          if (it_2->second == midiID)
            break;
        }
        if (it_2 == CNTR_PadHold.end())
          CNTR_PadMidiID.erase(midiID);
        // MLOGD("Midi Center","size of hold: %d. toggle: %d. ID: %d. midiIDCount: %d.", CNTR_PadHold.size(), CNTR_PadToggle.size(), CNTR_PadMidiID.size(), midiIDCount);
        return !CNTR_PadHold.empty();
      }
    }
    return !CNTR_PadHold.empty();
  }

  bool Scan_Arp()
  {
    for (auto it = CNTR_Arp.begin(); it != CNTR_Arp.end();)
    {
      uint16_t midiID = it->first;
      uint8_t channel = ID_Channel(midiID);
      uint8_t byte1 = ID_Byte1(midiID);
      if (MatrixOS::SYS::Millis() >= it->second)
      {
        MidiRouter(NODE_ARP, SEND_NOTE, channel, byte1, 0);
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

    for(auto it : CNTR_PadMidiID) {
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
  void ClearIDMap(std::unordered_map<TKey, TValue>& container, RouterNode node, bool eraseMidiID = true, int8_t type = -1, int8_t channel = -1) {
    if(container.empty()) return;

    for (auto it = container.begin(); it != container.end();) {
      uint16_t midiID = IDpos == MIDIID_IS_FIRST ? (uint16_t)it->first : (uint16_t)it->second;
      uint8_t ch = (midiID >> 8) & 0xF;
      if ((type == -1 || type == midiID >> 12) && (channel == -1 || ch == channel)) {
        uint8_t note = midiID & 0xFF;
        type = type == -1 ? midiID >> 12 : type;
        MidiRouter(node, type, ch, note, 0);
        if (eraseMidiID) CNTR_PadMidiID.erase(midiID);
        it = container.erase(it);
      } else {
        it++;
      }
    }
  }

  void ClearIDSet(std::unordered_set<uint16_t>& container, RouterNode node, bool eraseMidiID = true, int8_t type = -1, int8_t channel = -1) {
    if (container.empty()) return;

    for (auto it = container.begin(); it != container.end();)
    {
      uint8_t ch = (*it >> 8) & 0xF;
      if ((type == -1 || type == (*it >> 12)) && (channel == -1 || ch == channel)) {
        uint8_t note = *it & 0xFF;
        type = type == -1 ? *it >> 12 : type;
        MidiRouter(node, type, ch, note, 0);
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

  void NodeScan(){
    for (uint8_t ch = 0; ch < 16; ch++) {
      for (auto it = nodesInChannel[ch].begin(); it != nodesInChannel[ch].end(); it++) {
        it->second->Scan();
      }
    }
  }

  void ClockOut()
  {
    if(!transportState.play) timeReceived = false;
    if(clockIn && timeReceived) return;

    if(bpmPrv != bpm.byte2) 
    {
      tickInterval = (60000.0 / 24) / bpm.byte2; 
      bpmPrv = bpm.byte2;
      tickCount = (MatrixOS::SYS::Millis() - playStartTime) / tickInterval;
    }

    if(transportState.play && MatrixOS::SYS::Millis() >= playStartTime + (uint32_t)(tickInterval * tickCount)) {
      tickCount = (MatrixOS::SYS::Millis() - playStartTime) / tickInterval + 1;
      if (clockOut) MatrixOS::MIDI::Send(MidiPacket(0, Sync));
      if(tickCount % 8 == 0) 
      {
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
    tickInterval = (60000.0 / 24) / bpm.byte2;
    tickCount = 0;
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
  
  ChannelConfig* GetChannelConfig() { return channelConfig; }
  TransportState* TransState() { return &transportState; }
  Timer* GetBeatTimer() { return &beatTimer; }

  std::vector<KnobConfig*> GetSysKnobs() {return sysKnobs;}

  uint16_t GetMidiID(uint8_t type, uint8_t channel, uint8_t byte1) { return (type << 12) | (channel << 8) | byte1; }
  uint8_t ID_Type(uint16_t midiID) { return midiID >> 12; }
  uint8_t ID_Channel(uint16_t midiID) { return (midiID >> 8) & 0xF; }
  uint8_t ID_Byte1(uint16_t midiID) { return midiID & 0xFF; }
  

}