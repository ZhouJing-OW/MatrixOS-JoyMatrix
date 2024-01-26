#include "MatrixOS.h"
#include "MIDIHold.h"
#include <unordered_map>



namespace MatrixOS::MIDI
{

     std::unordered_map<uint16_t, uint16_t> hold; // KeyID , NoteID
     uint8_t afterTouch;

     void Hold(Point xy, int8_t type, int8_t channel, int8_t byte1, int8_t byte2){ //type 0:None, 1:CC, 2: PC, 3:Note
          switch(type) {
               case SEND_NONE: break; // None
               case SEND_CC: // CC
                    Send(MidiPacket(0, ControlChange, channel, byte1, byte2)); 
                    break;
               case SEND_PC: // PC
                    Send(MidiPacket(0, ControlChange, channel, 0, byte1));
                    Send(MidiPacket(0, ProgramChange, channel, byte2, byte2)); 
                    break;
               case SEND_NOTE: // Note
                    byte2 = Device::KeyPad::GetVelocity();
                    Send(MidiPacket(0, NoteOn, channel, byte1, byte2)); 
                    afterTouch = byte2;
                    break;
          }
          uint16_t keyID = Device::KeyPad::XY2ID(xy);
          uint16_t noteID = type << 12 | channel << 8 | byte1;
          hold.insert({keyID, noteID});
     }

     bool CheckHold(int8_t type, int8_t channel, int8_t byte1){
          uint16_t noteID = type << 12 | channel << 8 | byte1;
          for(auto it = hold.begin(); it != hold.end(); it++){
               if(it->second == noteID) return true;
          }
          return false;
     }

     bool CheckHold(Point xy){
          uint16_t keyID = Device::KeyPad::XY2ID(xy);
          auto it = hold.find(keyID);
          if(it != hold.end()){
               return true;
          }
          return false;
     }

     bool CheckHold(){ // Loop in UI objects
          for (auto it = hold.begin(); it != hold.end(); it++) {
               uint16_t midiID = it->second;
               uint8_t type = midiID >> 12;
               uint8_t channel = (midiID >> 8) & 0xF;
               uint8_t byte1 = midiID & 0xFF;
               uint8_t velocity = Device::KeyPad::GetVelocity();
               KeyInfo* keyInfo = Device::KeyPad::GetKey(it->first);

               if (keyInfo->state == AFTERTOUCH && type == SEND_NOTE){
                    if(Device::KeyPad::pressureSensitive){
                         if (velocity != afterTouch){
                              afterTouch = velocity;
                              Send(MidiPacket(0, AfterTouch, channel, byte1, afterTouch));
                              Send(MidiPacket(0, ControlChange, channel, 74, afterTouch));
                         }
                    }
               } 
               
               if(keyInfo->state == RELEASED || keyInfo->state == CLEARED || keyInfo->state == IDLE){
                    switch(type) {
                         case SEND_NONE: break;
                         case SEND_CC: 
                              Send(MidiPacket(0, ControlChange, channel, byte1, 0)); 
                              break;
                         case SEND_PC: break;
                         case SEND_NOTE:
                              Send(MidiPacket(0, NoteOff, channel, byte1, 0));
                              break;
                    }
                    hold.erase(it);
                    return !hold.empty();
               }
          }
          return !hold.empty();
     }
}