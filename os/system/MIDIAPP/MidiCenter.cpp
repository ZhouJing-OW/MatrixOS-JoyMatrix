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

  std::unordered_map<uint16_t, uint8_t*>       CNTR_FeedBack;
  
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

  TaskHandle_t midiCenterTaskHandle = NULL;
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
          case NoteOn:
          case NoteOff:
          {
            uint16_t midiID = MidiID(SEND_NOTE, midiPacket.data[0] & 0x0F, midiPacket.data[1]);
            auto it = CNTR_FeedBack.find(midiID);
            if (it != CNTR_FeedBack.end()) 
            {
              *it->second = midiPacket.data[2];
            }
            //MLOGD("Midi in", "Note: %d, %d, %d", midiPacket.data[0] & 0x0F, midiPacket.data[1], midiPacket.data[2]);
            break;
          }
          case ControlChange:
          {
            uint16_t midiID = MidiID(SEND_CC, midiPacket.data[0] & 0x0F, midiPacket.data[1]);
            auto it = CNTR_FeedBack.find(midiID);
            if (it != CNTR_FeedBack.end()) 
            {
              *it->second = midiPacket.data[2];
            }
            //MLOGD("Midi in", "CC: %d, %d, %d", midiPacket.data[0] & 0x0F, midiPacket.data[1], midiPacket.data[2]);
            break;
          }
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
      Scan_Retrig();
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

  void MidiCenterStart()
  {
    xTaskCreate(MidiCenterTask, "MidiCenter", configMINIMAL_STACK_SIZE * 6, NULL, configMAX_PRIORITIES - 5, &midiCenterTaskHandle);
    taskCount = 0;    
  }

  void MidiCenterStop()
  {
    if (midiCenterTaskHandle != NULL)
    {
        vTaskDelete(midiCenterTaskHandle);
        midiCenterTaskHandle = NULL;
    }    
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

  pair<uint16_t, uint8_t> FBButtons[128] = {
    {0x3000, 0}, {0x3001, 0}, {0x3002, 0}, {0x3003, 0}, {0x3004, 0}, {0x3005, 0}, {0x3006, 0}, {0x3007, 0},
    {0x3008, 0}, {0x3009, 0}, {0x300A, 0}, {0x300B, 0}, {0x300C, 0}, {0x300D, 0}, {0x300E, 0}, {0x300F, 0},
    {0x3010, 0}, {0x3011, 0}, {0x3012, 0}, {0x3013, 0}, {0x3014, 0}, {0x3015, 0}, {0x3016, 0}, {0x3017, 0},
    {0x3018, 0}, {0x3019, 0}, {0x301A, 0}, {0x301B, 0}, {0x301C, 0}, {0x301D, 0}, {0x301E, 0}, {0x301F, 0},

    {0x3020, 0}, {0x3021, 0}, {0x3022, 0}, {0x3023, 0}, {0x3024, 0}, {0x3025, 0}, {0x3026, 0}, {0x3027, 0},
    {0x3028, 0}, {0x3029, 0}, {0x302A, 0}, {0x302B, 0}, {0x302C, 0}, {0x302D, 0}, {0x302E, 0}, {0x302F, 0},
    {0x3030, 0}, {0x3031, 0}, {0x3032, 0}, {0x3033, 0}, {0x3034, 0}, {0x3035, 0}, {0x3036, 0}, {0x3037, 0},
    {0x3038, 0}, {0x3039, 0}, {0x303A, 0}, {0x303B, 0}, {0x303C, 0}, {0x303D, 0}, {0x303E, 0}, {0x303F, 0},

    {0x3040, 0}, {0x3041, 0}, {0x3042, 0}, {0x3043, 0}, {0x3044, 0}, {0x3045, 0}, {0x3046, 0}, {0x3047, 0},
    {0x3048, 0}, {0x3049, 0}, {0x304A, 0}, {0x304B, 0}, {0x304C, 0}, {0x304D, 0}, {0x304E, 0}, {0x304F, 0},
    {0x3050, 0}, {0x3051, 0}, {0x3052, 0}, {0x3053, 0}, {0x3054, 0}, {0x3055, 0}, {0x3056, 0}, {0x3057, 0},
    {0x3058, 0}, {0x3059, 0}, {0x305A, 0}, {0x305B, 0}, {0x305C, 0}, {0x305D, 0}, {0x305E, 0}, {0x305F, 0},

    {0x3060, 0}, {0x3061, 0}, {0x3062, 0}, {0x3063, 0}, {0x3064, 0}, {0x3065, 0}, {0x3066, 0}, {0x3067, 0},
    {0x3068, 0}, {0x3069, 0}, {0x306A, 0}, {0x306B, 0}, {0x306C, 0}, {0x306D, 0}, {0x306E, 0}, {0x306F, 0},
    {0x3070, 0}, {0x3071, 0}, {0x3072, 0}, {0x3073, 0}, {0x3074, 0}, {0x3075, 0}, {0x3076, 0}, {0x3077, 0},
    {0x3078, 0}, {0x3079, 0}, {0x307A, 0}, {0x307B, 0}, {0x307C, 0}, {0x307D, 0}, {0x307E, 0}, {0x307F, 0},
  };  // midiID, color

  pair<uint16_t, uint8_t> FBButtons2[8] = 
  {
    {0x3F00 ,0}, {0x3F01 ,0}, {0x3F02 ,0}, {0x3F03 ,0}, {0x3F04 ,0}, {0x3F05 ,0}, {0x3F06 ,0}, {0x3F07 ,0}
  };

  void RegistFeedBack()
  {
    for (int i = 0; i < 128; i++)
    {
      CNTR_FeedBack[FBButtons[i].first] = &FBButtons[i].second;
    }
    for (int i = 0; i < 8; i++)
    {
      CNTR_FeedBack[FBButtons2[i].first] = &FBButtons2[i].second;
    }
    // MLOGD("RegistFeedBack", "%d", CNTR_FeedBack.size());
  }

  void UnRegistFeedBack()
  {
    for (int i = 0; i < 128; i++)
    {
      CNTR_FeedBack.erase(FBButtons[i].first);
    }
    for (int i = 0; i < 8; i++)
    {
      CNTR_FeedBack.erase(FBButtons2[i].first);
    }
  }

  void ResetFeedBack()
  {
    for (int i = 0; i < 128; i++)
    {
      FBButtons[i].second = 0;
    }
    for (int i = 0; i < 8; i++)
    {
      FBButtons2[i].second = 0;
    }
  }

  uint16_t MidiID(uint8_t type, uint8_t channel, uint8_t byte1) { return (type << 12) | (channel << 8) | byte1; }
  uint8_t ID_Type(uint16_t midiID) { return midiID >> 12; }
  uint8_t ID_Channel(uint16_t midiID) { return (midiID >> 8) & 0xF; }
  uint8_t ID_Byte1(uint16_t midiID) { return midiID & 0xFF; }

  
  

}