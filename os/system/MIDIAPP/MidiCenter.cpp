#include "MidiCenter.h"

#define midi_center_scanrate 1000

namespace MatrixOS::MidiCenter
{
  TransportState transportState;
  KnobConfig bpm                = {.lock = true, .byte2 = MatrixOS::UserVar::BPM, .min = 20, .max = 300, .def = 120, .color = COLOR_PURPLE};
  KnobConfig swing              = {.lock = true, .byte2 = MatrixOS::UserVar::swing, .max = 100, .def = 50, .color = COLOR_GOLD};
  KnobConfig defaultVelocity    = {.lock = true, .byte2 = MatrixOS::UserVar::defaultVelocity, .min = 1, .def = 127, .color = COLOR_GREEN};
  std::vector<KnobConfig*> sysKnobs = {&bpm, &swing, &defaultVelocity};

  uint8_t afterTouch;
  Timer beatTimer;
  
  double tickInterval;
  uint32_t playStartTime;
  uint32_t tickCount;

  bool clockOut;
  bool clockIn;
  uint16_t bpmPrv;
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
            tickCount = 0;
            tickTime[0] = MatrixOS::SYS::Millis();
            MLOGD("Midi Center", " clock started");
            break;
          case Sync:
            if (!clockIn) break;

            tickCount++;
            if (tickCount % 8 == 0) {
              tickTime[tickCount / 8] = MatrixOS::SYS::Millis();
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
            tickCount = 0;
            MLOGD("Midi Center", " clock stoped");
            break;
          default:
            break;
        }
      }
      
      ClockOut();
      
      if (taskCount % 10 == 0) {// scan rate 100Hz
        Panic();
        HoldNoteMoveToToggle();
        Scan_Toggle();
        Scan_Hold();
        Scan_Arp();
        clockOut = MatrixOS::UserVar::clockOut;
        clockIn = MatrixOS::UserVar::clockIn;
      }
      
      taskCount++;
      if(taskCount >= 1000)
        taskCount = 0;
      
      vTaskDelayUntil(&xLastWakeTime, configTICK_RATE_HZ / midi_center_scanrate);
    }
  }

  void Init()
  {
    xTaskCreate(MidiCenterTask, "MidiCenter", configMINIMAL_STACK_SIZE * 2, NULL, configMAX_PRIORITIES - 5, NULL);
  }

  bool Scan_Toggle()
  {    
    if (toggle.empty()) return false;
    for (auto it = toggle.begin(); it != toggle.end(); it++)
    {
      uint8_t type = *it >> 12;
      if (Device::KeyPad::pressure_sensitive && type == SEND_NOTE)
      {
        uint8_t channel = (*it >> 8) & 0xF;
        uint8_t byte1 = *it & 0xFF;
        uint8_t velocity = Device::KeyPad::GetVelocity();
        if (velocity != afterTouch)
        {
          afterTouch = velocity;
          MatrixOS::MIDI::Send(MidiPacket(0, AfterTouch, channel, byte1, afterTouch));
        }                    
      }
    }
    return true;
  }

  bool Scan_Hold()
  {
    if(hold.empty()) return false;
    for (auto it = hold.begin(); it != hold.end(); it++) 
    {
      uint16_t midiID = it->second;
      uint8_t type = midiID >> 12;
      uint8_t channel = (midiID >> 8) & 0xF;
      uint8_t byte1 = midiID & 0xFF;
      KeyInfo* keyInfo = Device::KeyPad::GetKey(it->first);

      if (Device::KeyPad::pressure_sensitive && keyInfo->state == AFTERTOUCH && type == SEND_NOTE)
      {
        uint8_t velocity = Device::KeyPad::GetVelocity();
        if (velocity != afterTouch)
        {
          afterTouch = velocity;
          MatrixOS::MIDI::Send(MidiPacket(0, AfterTouch, channel, byte1, afterTouch));
        }
      } 
      
      if(keyInfo->state == RELEASED || keyInfo->state == CLEARED || keyInfo->state == IDLE)
      {
        MidiRouter(NODE_KEYPAD, type, channel, byte1, 0);
        hold.erase(it);
        auto it_2 = hold.begin();
        for( ; it_2 != hold.end(); it_2++)
        {
          if (it_2->second == midiID)
            break;
        }
        if (it_2 == hold.end())
          midiIDList.erase(midiID);
        // MLOGD("Midi Center","size of hold: %d. toggle: %d. ID: %d. midiIDCount: %d.", hold.size(), toggle.size(), midiIDList.size(), midiIDCount);
        return !hold.empty();
      }
    }
    return !hold.empty();
  }

  bool Scan_Arp()
  {
    for (auto it = arpeggiators.begin(); it != arpeggiators.end(); it++ )
      it->second->Scan();
    if (arp.empty()) return false;

    for (auto it = arp.begin(); it != arp.end();)
    {
      uint16_t midiID = it->first;
      uint8_t channel = (midiID >> 8) & 0xF;
      uint8_t byte1 = midiID & 0xFF;
      if (MatrixOS::SYS::Millis() >= it->second)
      {
        MidiRouter(NODE_ARP, SEND_NOTE, channel, byte1, 0);
        it = arp.erase(it);
      }
      else it++;
    }
    return !arp.empty();
  }

  void ClockOut()
  {
    if(clockIn) return;

    if(bpmPrv != bpm.byte2) 
    {
      tickInterval = (60000.0 / 24) / bpm.byte2; 
      bpmPrv = bpm.byte2;
      tickCount = (MatrixOS::SYS::Millis() - playStartTime) / tickInterval;
    }

    if(transportState.play && MatrixOS::SYS::Millis() >= playStartTime + (uint32_t)(tickInterval * tickCount)) {
      tickCount = (MatrixOS::SYS::Millis() - playStartTime) / tickInterval + 1;
      if (clockOut) MatrixOS::MIDI::Send(MidiPacket(0, Sync));
      if(tickCount % 24 == 0) 
      {
        MLOGD("Midi Center", "BPM: %d Interval: %f", bpm.byte2, tickInterval);
        beatTimer.RecordCurrent(); 
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
  
  TransportState* GetTransportState() { return &transportState; }

  Timer* GetBeatTimer() { return &beatTimer; }

  std::vector<KnobConfig*> GetSysKnobs() {return sysKnobs;}

}