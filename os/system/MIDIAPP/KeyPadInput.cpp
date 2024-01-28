#include "MidiCenter.h"

namespace MatrixOS::MidiCenter 
{
  std::map<uint16_t, uint16_t> hold; // keyID , midiID
  std::set<uint16_t> toggle; // midiID
  std::set<uint16_t> midiIDList; // midiID, midiIDCount

  void Hold(Point xy, int8_t type, int8_t channel, int8_t byte1, int8_t byte2) //type 0:None, 1:CC, 2: PC, 3:Note
  {    
    uint16_t keyID = Device::KeyPad::XY2ID(xy);
    uint16_t midiID = type << 12 | channel << 8 | byte1;
    auto it_m = midiIDList.find(midiID);
    if (it_m != midiIDList.end()) // The MIDI messages exist in the list
    {
      auto it_t = toggle.find(midiID);
      if (it_t != toggle.end()) // Exists in the toggle map
        toggle.erase(it_t);
      hold.insert({keyID, midiID});
    }
    else // Does not exist
    {
      hold.insert({keyID, midiID});
      midiIDList.insert(midiID);
    }
    MidiRouter(NODE_KEYPAD, type, channel, byte1, byte2);
    // MLOGD("Midi Center","size of hold: %d. toggle: %d. ID: %d. midiIDCount: %d.", hold.size(), toggle.size(), midiIDList.size(), midiIDCount);
  }

  void Toggle(int8_t type, int8_t channel, int8_t byte1, int8_t byte2)
  {
    uint16_t midiID = type << 12 | channel << 8 | byte1;
    auto it_m = midiIDList.find(midiID);
    if (it_m != midiIDList.end()) // the MIDI messages exist in the list
    {
      auto it_t = toggle.find(midiID);
      if (it_t != toggle.end()) // Exists in the toggle map
      {
        toggle.erase(it_t);
        midiIDList.erase(midiID);
        MidiRouter(NODE_KEYPAD, type, channel, byte1, 0);
      }
    } 
    else // Does not exist
    {
      if(type != SEND_PC) 
      {
      toggle.insert(midiID);
      midiIDList.insert(midiID);
      }
      MidiRouter(NODE_KEYPAD, type, channel, byte1, byte2);
    }
    // MLOGD("Midi Center","size of hold: %d. toggle: %d. ID: %d. midiIDCount: %d.", hold.size(), toggle.size(), midiIDList.size(), midiIDCount);
  }

  bool FindArp(int8_t type, int8_t channel, int8_t byte1)
  {
    uint16_t midiID = type << 12 | channel << 8 | byte1;
    auto it = arp.find(midiID);
    if (it != arp.end()) return true;
    return false;
  }

  bool FindHold(int8_t type, int8_t channel, int8_t byte1)
  {
    uint16_t midiID = type << 12 | channel << 8 | byte1;
    auto it = midiIDList.find(midiID);
    if (it != midiIDList.end()) return true;
    return false;
  }

  bool FindHold(Point xy)
  {
    uint16_t keyID = Device::KeyPad::XY2ID(xy);
    auto it = hold.find(keyID);
    if(it != hold.end()) return true; 
    return false;
  }

  bool moveToToggle;
  void HoldNoteMoveToToggle() // when shift is pressed, move hold note to toggle
  {
    if(Device::KeyPad::Shift() && moveToToggle == false) { moveToToggle = true; }
    else { moveToToggle = false; return; }
      
    uint8_t activeChannel = MatrixOS::UserVar::global_channel;
    if (!hold.empty() && !toggle.empty()) // clear previous toggle note in the active channel
    {
      for(auto it = toggle.begin(); it != toggle.end();)
      {
        uint8_t type = *it >> 12;
        uint8_t channel = (*it >> 8) & 0xF;
        if(type == SEND_NOTE && channel == activeChannel) 
        {
          uint8_t note = *it & 0xFF;
          midiIDList.erase(*it);
          it = toggle.erase(it);
          MidiRouter(NODE_KEYPAD, type, channel, note, 0);
        }
        else it++;
      }
    }

    for(auto it = hold.begin(); it != hold.end();) // move hold note to toggle
    { 
      uint8_t type = it->second >> 12;
      if(type == SEND_NOTE) 
      {
        toggle.insert(it->second);
        it = hold.erase(it);
      }
      else it++;
    }
  }

  void ClearHold()
  {
    for (auto it = hold.begin(); it != hold.end(); )
    {
      uint8_t type = it->second >> 12;
      uint8_t channel = (it->second >> 8) & 0xF;
      uint8_t byte1 = it->second & 0xFF;
      MidiRouter(NODE_KEYPAD, type, channel, byte1, 0);
      midiIDList.erase(it->second);
      it = hold.erase(it);
    }
  }

  void ClearToggle()
  {
    for (auto it = toggle.begin(); it != toggle.end(); )
    {
      uint8_t type = *it >> 12;
      uint8_t channel = (*it >> 8) & 0xF;
      uint8_t byte1 = *it & 0xFF;
      MidiRouter(NODE_KEYPAD, type, channel, byte1, 0);
      midiIDList.erase(*it);
      it = toggle.erase(it);
    }
  }

  Timer panicTimer;
  uint8_t panicStep = 0;
  void Panic() // when both shift is pressed, panic
  {
    if(Device::KeyPad::BothShift())  {
      if(panicStep < 4 && panicTimer.IsLonger(1500)) {
        panicStep++;
        panicTimer.RecordCurrent();
      }
    }
    else
    {
      panicStep = 0;
      return;
    }
      
    switch (panicStep)
    {
      case 0:
        break;
      case 1: // Panic Current Channel
      {
        int8_t globalChannel = MatrixOS::UserVar::global_channel;

        for (auto it = toggle.begin(); it != toggle.end();) // toggle.panic
        {
          uint8_t type = *it >> 12;
          uint8_t channel = (*it >> 8) & 0xF;
          if (type == SEND_NOTE && channel == globalChannel) 
          {
            uint8_t note = *it & 0xFF;
            MidiRouter(NODE_KEYPAD, type, channel, note, 0);
            midiIDList.erase(*it);
            it = toggle.erase(it); 
            MatrixOS::SYS::DelayMs(1);
            // MLOGD("Midi Center","size of hold: %d. toggle: %d. ID: %d. midiIDCount: %d.", hold.size(), toggle.size(), midiIDList.size(), midiIDCount);
          }
          else it++;
        }

        for (auto it = hold.begin(); it != hold.end(); ) // hold.panic
        {    
          uint8_t type = it->second >> 12;
          uint8_t channel = (it->second >> 8) & 0xF;
          if (type == SEND_NOTE && channel == globalChannel)
          {
            uint8_t note = it->second & 0xFF;
            MidiRouter(NODE_KEYPAD, type, channel, note, 0);
            midiIDList.erase(it->second);
            it = hold.erase(it);
            // MLOGD("Midi Center","size of hold: %d. toggle: %d. ID: %d. midiIDCount: %d.", hold.size(), toggle.size(), midiIDList.size(), midiIDCount);
          }
          else it++;
        }
        
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, globalChannel, 123, 127));
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, globalChannel, 123, 0));

        break;
      }
      case 2: // Panic all channel
      {
        for (auto it = toggle.begin(); it != toggle.end();) // toggle.panic
        {
          uint8_t type = *it >> 12;
          if (type == SEND_NOTE) 
          {
            uint8_t channel = (*it >> 8) & 0xF;
            uint8_t note = *it & 0xFF;
            MidiRouter(NODE_KEYPAD, type, channel, note, 0);
            midiIDList.erase(*it);
            it = toggle.erase(it); 
            // MLOGD("Midi Center","size of hold: %d. toggle: %d. ID: %d. midiIDCount: %d.", hold.size(), toggle.size(), midiIDList.size(), midiIDCount);
          }
          else it++;
        }

        for (auto it = hold.begin(); it != hold.end(); ) // hold.panic
        {    
          uint8_t type = it->second >> 12;
          if (type == SEND_NOTE)
          {
            uint8_t channel = (it->second >> 8) & 0xF;
            uint8_t note = it->second & 0xFF;
            MidiRouter(NODE_KEYPAD, type, channel, note, 0);
            midiIDList.erase(it->second);
            it = hold.erase(it);
            // MLOGD("Midi Center","size of hold: %d. toggle: %d. ID: %d. midiIDCount: %d.", hold.size(), toggle.size(), midiIDList.size(), midiIDCount);
          }
          else it++;
        }
        
        for (uint8_t ch = 0; ch < 16; ch++) { // channel panic
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, ch, 123, 127));
          MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, ch, 123, 0));
        }

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