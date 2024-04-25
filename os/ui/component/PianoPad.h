// #pragma once

// #include "UIComponent.h"
// #include "MatrixOS.h"

// class PianoPad : public UIComponent {
//  public:
//   Dimension dimension;
//   NotePadConfig* config;
//   Point position = Point(0, 0);
  
//   uint32_t octaveTimer;
//   int8_t lastOctave;
//   bool octaveDisplay;

//   Color color = COLOR_PIANO_PAD[0];
//   Color rootColor = COLOR_PIANO_PAD[1];


//   const uint16_t octaveDisplayDuration = 200;
  
//   PianoPad(Dimension dimension, NotePadConfig* config) {
//     this->dimension = dimension;
//     this->config = config;
//     this->lastOctave = config->octave;
//     octaveDisplay = false;
//   }

//   virtual Color GetColor() { return rootColor; }
//   virtual Dimension GetSize() { return dimension; }
  
//   const int8_t pianoNote[2][7] = {{ -1 , 1 , 3 , -1 , 6 , 8 , 10 },
//                                   {  0 , 2 , 4 ,  5 , 7 , 9 , 11 }};

//   virtual bool Render(Point origin) {
//     position = origin;
//     uint16_t c_aligned_scale_map = ((config->scale << config->rootKey) + ((config->scale & 0xFFF) >> (12 - config->rootKey % 12))) & 0xFFF;  // Rootkey should always < 12,
//                                                                                                                                              // might add an assert later

//     if (dimension.x >= 10){
//       if (lastOctave != config->octave) {
//         octaveTimer = MatrixOS::SYS::Millis() + octaveDisplayDuration; 
//         lastOctave = config->octave;
//         octaveDisplay = true;
//       }
//       if (octaveTimer < MatrixOS::SYS::Millis()) {
//         octaveDisplay = false; octaveTimer = 0; 
//       }
//     }

//     if(octaveDisplay) {
//       uint16_t octaveX = (dimension.x) / 2 - 5;
//       for (int8_t y = 0; y < dimension.y; y++)
//       {
//         for (int8_t x = 0; x < dimension.x; x++)
//         {
//           Point xy = origin + Point(x, y);
//           if (x >= octaveX && x < octaveX + 10 && y == dimension.y - 1) {
//             if (x == octaveX + config->octave) {
//               MatrixOS::LED::SetColor(xy, Color(WHITE));
//             } else {
//               MatrixOS::LED::SetColor(xy, color.ToLowBrightness());
//             }
//           } else {
//             MatrixOS::LED::SetColor(xy, Color(BLANK));
//           }
//         }
//       }
//       return true;
//     }

//     if(!octaveDisplay) {
//       for(uint8_t x = 0; x < dimension.x; x++){
//         for(uint8_t y = 0; y < dimension.y; y++){
//           int8_t octaveShift = x - config->shift < 0 ? -1 : ((x - config->shift) / 7);
//           int8_t octave = config->octave + octaveShift + ((dimension.y - y - 1) / 2) * (dimension.x / 7);
//           int8_t note = pianoNote[(dimension.y + y) % 2][(x - config->shift + 7) % 7] + octave * 12;
//           Point xy = origin + Point(x, y);
//           int8_t channel = config->globalChannel ? MatrixOS::UserVar::global_channel : config->channel;

//           if(pianoNote[(dimension.y + y) % 2][(x - config->shift + 7) % 7] >= 0 && note >= 0){
//             if (MatrixOS::MidiCenter::FindHold(SEND_NOTE, channel, note)) {  // If find the note is currently active. Show it as white
//               MatrixOS::LED::SetColor(xy, Color(WHITE)); 
//             } else if (note % 12 == config->rootKey) {
//               MatrixOS::LED::SetColor(xy, rootColor.Blink_Key(Device::KeyPad::fnState));
//             } else if (bitRead(c_aligned_scale_map, pianoNote[(dimension.y + y) % 2][(x - config->shift + 7) % 7])) {
//               MatrixOS::LED::SetColor(xy, color.Blink_Key(Device::KeyPad::fnState));
//             } else {
//               Color thisColor = color.ToLowBrightness();
//               MatrixOS::LED::SetColor(xy, thisColor.Blink_Key(Device::KeyPad::fnState));
//             }
//           } else 
//             MatrixOS::LED::SetColor(xy, Color(BLANK))));
//         }
//       }
//       return true;
//     }
//     return false;
//   }

//   virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
//     int8_t octaveShift = xy.x - config->shift < 0 ? -1 : ((xy.x - config->shift) / 7);
//     int8_t octave = config->octave + octaveShift + ((dimension.y - xy.y - 1) / 2) * (dimension.x / 7); 
//     int8_t note = pianoNote[(dimension.y + xy.y) % 2][(xy.x - config->shift + 7) % 7] + octave * 12;
//     int8_t channel = config->globalChannel ? MatrixOS::UserVar::global_channel : config->channel;

//     if(!octaveDisplay){
//       if (pianoNote[(dimension.y + xy.y) % 2][(xy.x - config->shift + 7) % 7] == -1)
//         return false; 
//       else if (note < 0)
//         return false; 
//       else if (keyInfo->state == PRESSED) {
//         if(Device::KeyPad::fnState.active()) {
//           MatrixOS::Component::Pad_Setting(config, PIANO_PAD);
//           return true;
//         }
//         if(Device::KeyPad::Shift()) MatrixOS::MidiCenter::Toggle(SEND_NOTE, channel, note);
//         else MatrixOS::MidiCenter::Hold(xy + position, SEND_NOTE, channel, note);
//       } 
//       return true;
//     }
//     return false;
//   }
// };