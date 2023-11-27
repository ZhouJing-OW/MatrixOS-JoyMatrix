#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class PianoPad : public UIComponent {
 public:
  int8_t octaveOffset;
  Dimension dimension;
  NotePadConfig* config;
  Point position = Point(0, 0);

  PianoPad(Dimension dimension, int8_t octaveOffset, NotePadConfig* config) {
    this->dimension = dimension;
    this->octaveOffset = octaveOffset;
    this->config = config;
  }

  virtual Color GetColor() { return config->color; }
  virtual Dimension GetSize() { return dimension; }
  
  const int8_t pianoNote[2][7] = {{ -1 , 1 , 3 , -1 , 6 , 8 , 10 },
                                  {  0 , 2 , 4 ,  5 , 7 , 9 , 11 }};

  virtual bool Render(Point origin) {
    position = origin;
    uint16_t c_aligned_scale_map = ((config->scale << config->rootKey) + ((config->scale & 0xFFF) >> (12 - config->rootKey % 12))) & 0xFFF;  // Rootkey should always < 12,
                                                                                                                                             // might add an assert later
    for(uint8_t x = 0; x < dimension.x; x++){
      for(uint8_t y = 0; y < dimension.y; y++){
        uint8_t octave = config->octave + octaveOffset + (x / 7) + ((dimension.y - y - 1) / 2) * (dimension.x / 7);
        uint8_t note = pianoNote[(dimension.y + y) % 2][x % 7] + octave * 12;
        Point xy = origin + Point(x, y);

        if(pianoNote[(dimension.y + y) % 2][x % 7] >= 0 && note <= 127){
          if (MatrixOS::MIDI::CheckHoldingNote(config->channel, note)) {  // If find the note is currently active. Show it as white
            MatrixOS::LED::SetColor(xy, COLOR_WHITE); 
          } else if (note % 12 == config->rootKey) {
            MatrixOS::LED::SetColor(xy, config->rootColor);
          } else if (bitRead(c_aligned_scale_map, pianoNote[(dimension.y + y) % 2][x % 7])) {
            MatrixOS::LED::SetColor(xy, config->color);
          } else {
            MatrixOS::LED::SetColor(xy, config->color.ToLowBrightness());
          }
        } else 
          MatrixOS::LED::SetColor(xy, COLOR_BLANK);
      }
    }
    return true;
  }
  
  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    uint8_t octave = config->octave + octaveOffset + (xy.x / 7) + ((dimension.y - xy.y - 1) / 2) * (dimension.x / 7); 
    uint8_t note = pianoNote[(dimension.y + xy.y) % 2][xy.x % 7] + octave * 12;

    if (pianoNote[(dimension.y + xy.y) % 2][xy.x % 7] == -1)
      return false; 
    else if (note > 127)
      return false; 
    else if (keyInfo->state == PRESSED) {
      MatrixOS::MIDI::HoldNote(config->channel, note, xy + position);
    }
    return true;
  }
  
};