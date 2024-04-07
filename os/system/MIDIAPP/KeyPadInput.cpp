#include "MidiCenter.h"

namespace MatrixOS::MidiCenter 
{
  std::unordered_map<uint16_t, uint16_t> CNTR_PadHold; // keyID , midiID
  std::unordered_set<uint16_t> CNTR_PadToggle; // midiID
  std::unordered_set<uint16_t> CNTR_PadMidiID; // midiID, midiIDCount
  PadCheck padCheck[127];
  uint8_t inputCount = 0;

  void Hold(Point xy, int8_t type, int8_t channel, int8_t byte1, int8_t byte2) //type 0:None, 1:CC, 2: PC, 3:Note
  { 
    if (inputCount >= 16) return; // 1 second 16 notes limit
    inputCount++;
    
    uint16_t keyID = Device::KeyPad::XY2ID(xy);
    uint16_t midiID = GetMidiID(type, channel, byte1);
    auto it_m = CNTR_PadMidiID.find(midiID);
    if (it_m != CNTR_PadMidiID.end()) // The MIDI messages exist in the list
    {
      auto it_t = CNTR_PadToggle.find(midiID);
      if (it_t != CNTR_PadToggle.end()) // Exists in the toggle map
        CNTR_PadToggle.erase(it_t);
      CNTR_PadHold.insert({keyID, midiID});
    }
    else // Does not exist
    {
      CNTR_PadHold.insert({keyID, midiID});
      CNTR_PadMidiID.insert(midiID);
    }
    MidiRouter(NODE_KEYPAD, type, channel, byte1, byte2);
    // MLOGD("Midi Center","size of hold: %d. toggle: %d. ID: %d. midiIDCount: %d.", CNTR_PadHold.size(), CNTR_PadToggle.size(), CNTR_PadMidiID.size(), midiIDCount);
  }

  void Toggle(int8_t type, int8_t channel, int8_t byte1, int8_t byte2)
  {
    if(inputCount >= 16) return; // 1 second 16 notes limit
    inputCount++;

    uint16_t midiID = GetMidiID(type, channel, byte1);
    auto it_m = CNTR_PadMidiID.find(midiID);
    if (it_m != CNTR_PadMidiID.end()) // the MIDI messages exist in the list
    {
      auto it_t = CNTR_PadToggle.find(midiID);
      if (it_t != CNTR_PadToggle.end()) // Exists in the toggle map
      {
        CNTR_PadToggle.erase(it_t);
        CNTR_PadMidiID.erase(midiID);
        MidiRouter(NODE_KEYPAD, type, channel, byte1, 0);
      }
    } 
    else // Does not exist
    {
      if(type != SEND_PC) 
      {
      CNTR_PadToggle.insert(midiID);
      CNTR_PadMidiID.insert(midiID);
      }
      MidiRouter(NODE_KEYPAD, type, channel, byte1, byte2);
    }
    // MLOGD("Midi Center","size of hold: %d. toggle: %d. ID: %d. midiIDCount: %d.", CNTR_PadHold.size(), CNTR_PadToggle.size(), CNTR_PadMidiID.size(), midiIDCount);
  }

  bool FindChord(int8_t channel, int8_t byte1)
  {
    uint16_t midiID = GetMidiID(SEND_NOTE, channel, byte1);
    if (CNTR_Chord.find(midiID) != CNTR_Chord.end()) return true;
    return false;
  }

  bool FindArp(int8_t channel, int8_t byte1)
  {
    uint16_t midiID = GetMidiID(SEND_NOTE, channel, byte1);
    if (CNTR_Arp.find(midiID) != CNTR_Arp.end()) return true;
    return false;
  }

  bool FindHold(int8_t type, int8_t channel, int8_t byte1)
  {
    uint16_t midiID = GetMidiID(type, channel, byte1);
    if (CNTR_PadMidiID.find(midiID) != CNTR_PadMidiID.end()) return true;
    return false;
  }

  bool FindHold(Point xy)
  {
    uint16_t keyID = Device::KeyPad::XY2ID(xy);
    if(CNTR_PadHold.find(keyID) != CNTR_PadHold.end()) return true; 
    return false;
  }

  PadCheck GetPadCheck(int8_t byte1) { return padCheck[byte1]; }

  bool moveToToggle;
  void MoveHoldToToggle() // when shift is pressed, move hold note to toggle
  {
    KnobConfig* dial = Device::AnalogInput::GetDialPtr();
    if(Device::KeyPad::Shift() && dial == nullptr && moveToToggle == false) { moveToToggle = true; }
    else { moveToToggle = false; return; }
      
    uint8_t activeChannel = MatrixOS::UserVar::global_channel;
    if (!CNTR_PadHold.empty() && !CNTR_PadToggle.empty()) // clear previous toggle note in the active channel
    {
      for(auto it = CNTR_PadToggle.begin(); it != CNTR_PadToggle.end();)
      {
        uint8_t type = ID_Type(*it);
        uint8_t channel = ID_Channel(*it);
        if(type == SEND_NOTE && channel == activeChannel) 
        {
          uint8_t note = ID_Byte1(*it);
          CNTR_PadMidiID.erase(*it);
          it = CNTR_PadToggle.erase(it);
          MidiRouter(NODE_KEYPAD, type, channel, note, 0);
        }
        else it++;
      }
    }

    for(auto it = CNTR_PadHold.begin(); it != CNTR_PadHold.end();) // move hold note to toggle
    { 
      uint8_t type = ID_Type(it->second);
      if(type == SEND_NOTE) 
      {
        CNTR_PadToggle.insert(it->second);
        it = CNTR_PadHold.erase(it);
      }
      else it++;
    }
  }

  void PanicChannel(int8_t channel = -1) {
    ClearHold(SEND_NOTE, channel);
    ClearToggle(SEND_NOTE, channel);
    ClearArp(channel);

    uint8_t ch_start = channel == -1 ? 0 : channel;
    uint8_t ch_end = channel == -1 ? 16 : channel + 1;
    for (uint8_t ch = ch_start; ch < ch_end; ch++) {
      MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, ch, 123, 127));
      MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, ch, 123, 0));
    }
  }

  Timer panicTimer;
  uint8_t panicStep = 0;
  void Panic() {
    if(Device::KeyPad::BothShift())  {
      if(panicStep < 4 && panicTimer.IsLonger(1500)) {
          panicStep++;
          panicTimer.RecordCurrent();
      }
    } else {
      panicStep = 0;
      return;
    }

    switch (panicStep) {
      case 0:
        break;
      case 1: // Panic Current Channel
      {
        int8_t globalChannel = MatrixOS::UserVar::global_channel;
        PanicChannel(globalChannel);
        break;
      }
      case 2: // Panic all channel
      {
        PanicChannel();
        break;
      }
      case 3:
        for(uint8_t ch = 0; ch < 16; ch++) {
          for(uint8_t n = 0; n < 128; n++) {
            MatrixOS::MIDI::Send(MidiPacket(0, NoteOff, ch, n, 0));
            if (!Device::KeyPad::BothShift()) break;
            if (n % 16 == 0) MatrixOS::SYS::DelayMs(5);
          }
        }
        break;
      default:
        break;
    }
  }

}