#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class PianoPad : public UIComponent {
 public:
  int8_t octaveOffset;
  Dimension dimension;
  NotePadConfig* config;
  Point position = Point(0, 0);
  
  uint32_t octaveTimer;
  int8_t lastOctave;
  bool octaveDisplay;
  bool changeRoot = false;

  const uint16_t octaveDisplayDuration = 200;
  

  PianoPad(Dimension dimension, NotePadConfig* config, bool changeRoot = false, int8_t octaveOffset = 0) {
    this->dimension = dimension;
    this->octaveOffset = octaveOffset;
    this->config = config;
    this->changeRoot = changeRoot;
    this->lastOctave = config->octave;
    octaveDisplay = false;
  }

  virtual Color GetColor() { return config->color; }
  virtual Dimension GetSize() { return dimension; }
  
  const int8_t pianoNote[2][7] = {{ -1 , 1 , 3 , -1 , 6 , 8 , 10 },
                                  {  0 , 2 , 4 ,  5 , 7 , 9 , 11 }};

  virtual bool Render(Point origin) {
    position = origin;
    uint16_t c_aligned_scale_map = ((config->scale << config->rootKey) + ((config->scale & 0xFFF) >> (12 - config->rootKey % 12))) & 0xFFF;  // Rootkey should always < 12,
                                                                                                                                             // might add an assert later

    if (dimension.x >= 10){
      if (lastOctave != config->octave) {
        octaveTimer = MatrixOS::SYS::Millis() + octaveDisplayDuration; 
        lastOctave = config->octave;
        octaveDisplay = true;
      }
      if (octaveTimer < MatrixOS::SYS::Millis()) {
        octaveDisplay = false; octaveTimer = 0; 
      }
    }

    if(octaveDisplay) {
      uint16_t octaveX = (dimension.x) / 2 - 5;
      for (int8_t y = 0; y < dimension.y; y++)
      {
        for (int8_t x = 0; x < dimension.x; x++)
        {
          Point xy = origin + Point(x, y);
          if (x >= octaveX && x < octaveX + 10 && y == dimension.y - 1) {
            if (x == octaveX + config->octave) {
              MatrixOS::LED::SetColor(xy, COLOR_WHITE);
            } else {
              MatrixOS::LED::SetColor(xy, config->color.ToLowBrightness());
            }
          } else {
            MatrixOS::LED::SetColor(xy, COLOR_BLANK);
          }
        }
      }
      return true;
    }

    if(!octaveDisplay) {
      for(uint8_t x = 0; x < dimension.x; x++){
        for(uint8_t y = 0; y < dimension.y; y++){
          uint8_t octave = config->octave + octaveOffset + (x / 7) + ((dimension.y - y - 1) / 2) * (dimension.x / 7);
          uint8_t note = pianoNote[(dimension.y + y) % 2][x % 7] + octave * 12;
          Point xy = origin + Point(x, y);
          uint8_t channel = config->globalChannel ? MatrixOS::UserVar::global_MIDI_CH : config->channel;

          if(pianoNote[(dimension.y + y) % 2][x % 7] >= 0 && note <= 127){
            if (MatrixOS::MIDI::CheckHold(SEND_NOTE, channel, note)) {  // If find the note is currently active. Show it as white
              MatrixOS::LED::SetColor(xy, COLOR_WHITE); 
            } else if (note % 12 == config->rootKey) {
              MatrixOS::LED::SetColor(xy, changeRoot ? config->rootColor : config->rootColor.Blink(Device::KeyPad::fnState));
            } else if (bitRead(c_aligned_scale_map, pianoNote[(dimension.y + y) % 2][x % 7])) {
              MatrixOS::LED::SetColor(xy, changeRoot ? config->color : config->color.Blink(Device::KeyPad::fnState));
            } else {
              MatrixOS::LED::SetColor(xy, changeRoot ? config->color.ToLowBrightness() : config->color.ToLowBrightness().Blink(Device::KeyPad::fnState));
            }
          } else 
            MatrixOS::LED::SetColor(xy, COLOR_BLANK);
        }
      }
      return true;
    }
    return false;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    uint8_t octave = config->octave + octaveOffset + (xy.x / 7) + ((dimension.y - xy.y - 1) / 2) * (dimension.x / 7); 
    uint8_t note = pianoNote[(dimension.y + xy.y) % 2][xy.x % 7] + octave * 12;
    uint8_t channel = config->globalChannel ? MatrixOS::UserVar::global_MIDI_CH : config->channel;

    if(!octaveDisplay){
      if (pianoNote[(dimension.y + xy.y) % 2][xy.x % 7] == -1)
        return false; 
      else if (note > 127)
        return false; 
      else if (keyInfo->state == PRESSED) {
        if((Device::KeyPad::fnState == ACTIVATED || Device::KeyPad::fnState == HOLD) && changeRoot == false) {
          MatrixOS::Component::Pad_Setting(config);
          return true;
        }
        if(changeRoot) config->rootKey = note % 12;
        MatrixOS::MIDI::Hold(xy + position, SEND_NOTE, channel, note);
      } 
      return true;
    }
    return false;
  }
};