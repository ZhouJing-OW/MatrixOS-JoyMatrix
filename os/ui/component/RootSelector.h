#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class RootSelector : public UIComponent {
 public:
  int8_t octaveOffset;
  Dimension dimension;
  NotePadConfig* config;
  Point position = Point(0, 0);
  const uint16_t octaveDisplayDuration = 200;
  Color color[2] = {COLOR_PIANO_PAD[0], COLOR_PIANO_PAD[1]};

  RootSelector (Dimension dimension, NotePadConfig* config, int8_t octaveOffset = 0, int8_t type = PIANO_PAD) {
    this->dimension = dimension;
    this->config = config;
    this->octaveOffset = octaveOffset;
  }

  virtual Color GetColor() { return color[1]; }
  virtual Dimension GetSize() { return dimension; }
  
  const int8_t pianoNote[2][7] = {{ -1 , 1 , 3 , -1 , 6 , 8 , 10 },
                                  {  0 , 2 , 4 ,  5 , 7 , 9 , 11 }};

  virtual bool Render(Point origin) {
    position = origin;
    uint16_t c_aligned_scale_map = ((config->scale << config->rootKey) + ((config->scale & 0xFFF) >> (12 - config->rootKey % 12))) & 0xFFF;  // Rootkey should always < 12,
                                                                                                                                             // might add an assert later
    for(uint8_t x = 0; x < dimension.x; x++)
    {
      for(uint8_t y = 0; y < dimension.y; y++)
      {
        uint8_t octave = 3 + octaveOffset + (x / 7) + ((dimension.y - y - 1) / 2) * (dimension.x / 7);
        uint8_t note = pianoNote[(dimension.y + y) % 2][x % 7] + octave * 12;
        Point xy = origin + Point(x, y);

        if(pianoNote[(dimension.y + y) % 2][x % 7] >= 0 && note <= 127)
        {
          if (note % 12 == config->rootKey)
            MatrixOS::LED::SetColor(xy, color[1]);
          else if (bitRead(c_aligned_scale_map, pianoNote[(dimension.y + y) % 2][x % 7]))
            MatrixOS::LED::SetColor(xy, color[0]);
          else
          {
            MatrixOS::LED::SetColor(xy, color[0].DimIfNot());
          }
        } else 
          MatrixOS::LED::SetColor(xy, Color(BLANK));
      }
    }
    return true;
  }

  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    uint8_t octave = 3 + octaveOffset + (xy.x / 7) + ((dimension.y - xy.y - 1) / 2) * (dimension.x / 7); 
    uint8_t note = pianoNote[(dimension.y + xy.y) % 2][xy.x % 7] + octave * 12;
    uint8_t channel = MatrixOS::UserVar::global_channel;

    if (pianoNote[(dimension.y + xy.y) % 2][xy.x % 7] == -1)
      return false; 
    else if (note > 127)
      return false; 
    else if (keyInfo->state == PRESSED) {
      config->rootKey = note % 12;
      MatrixOS::MidiCenter::Hold(xy + position, SEND_NOTE, channel, note);
      return true;
    } 
    if (keyInfo->state == HOLD){ return true; }
    return false;
  }
};