#include "MatrixOS.h"
#include <unordered_map>


namespace MatrixOS::MIDI
{

     std::unordered_map<uint16_t, uint16_t> holdNotes; // KeyID , NoteID
     // holdNotes.reserve(10);
     uint8_t afterTouch;

     void HoldNote(uint8_t channel, uint8_t note, Point xy){
          uint8_t velocity = Device::KeyPad::GetVelocity();
          Send(MidiPacket(0, NoteOn, channel, note, velocity));
          uint16_t keyID = Device::KeyPad::XY2ID(xy);
          uint16_t noteID = channel << 8 | note;
          holdNotes.insert({keyID, noteID});
          afterTouch = velocity;
          MLOGD("Midi", "HoldNote: channel: %d, note: %d", channel, note);
     }

     bool CheckHoldingNote(uint8_t channel, uint8_t note){
          uint16_t noteID = channel << 8 | note;
          for(auto it = holdNotes.begin(); it != holdNotes.end(); it++){
               if(it->second == noteID){
                    return true;
               }
          }
          return false;
     }

     bool CheckHoldingNote(Point xy){
          uint16_t keyID = Device::KeyPad::XY2ID(xy);
          auto it = holdNotes.find(keyID);

          if(it != holdNotes.end()){
               uint16_t noteID = it->second;
               uint8_t channel = noteID >> 8;
               uint8_t note = noteID & 0xFF;
               uint8_t velocity = Device::KeyPad::GetVelocity();
               KeyInfo* keyInfo = Device::KeyPad::GetKey(keyID);

               if (keyInfo->state == AFTERTOUCH){
                    if(Device::KeyPad::pressureSensitive){
                         if (velocity != afterTouch){
                              afterTouch = velocity;
                              Send(MidiPacket(0, AfterTouch, channel, note, afterTouch));
                              Send(MidiPacket(0, ControlChange, channel, 74, afterTouch));
                         }
                    }
                    return true;
               } 
               
               if(keyInfo->state == RELEASED || keyInfo->state == CLEARED || keyInfo->state == IDLE){
                    Send(MidiPacket(0, NoteOff, channel, note, 0));
                    holdNotes.erase(it);
                    MLOGD("Midi", "Release Note: channel: %d, note: %d", channel, note);
                    return false;
               }

          }
          return false;
     }

     bool CheckHoldingNote(){
          for (auto it = holdNotes.begin(); it != holdNotes.end(); it++) {
               uint16_t noteID = it->second;
               uint8_t channel = noteID >> 8;
               uint8_t note = noteID & 0xFF;
               uint8_t velocity = Device::KeyPad::GetVelocity();
               KeyInfo* keyInfo = Device::KeyPad::GetKey(it->first);

               if (keyInfo->state == AFTERTOUCH){
                    if(Device::KeyPad::pressureSensitive){
                         if (velocity != afterTouch){
                              afterTouch = velocity;
                              Send(MidiPacket(0, AfterTouch, channel, note, afterTouch));
                              Send(MidiPacket(0, ControlChange, channel, 74, afterTouch));
                         }
                    }
               } 
               
               if(keyInfo->state == RELEASED || keyInfo->state == CLEARED || keyInfo->state == IDLE){
                    Send(MidiPacket(0, NoteOff, channel, note, 0));
                    holdNotes.erase(it);
                    MLOGD("Midi", "Release Note: channel: %d, note: %d", channel, note);
                    return !holdNotes.empty();
               }
          }
          return !holdNotes.empty();
     }
}