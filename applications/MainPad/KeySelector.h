#pragma once

#include "MatrixOS.h"

class KeySelector : public UIComponent {
 public:
  uint8_t* value;
  uint8_t* octave;
  uint8_t group;
  uint8_t number;
  ButtonConfig* config;

  KeySelector(uint8_t* octave, uint8_t group, uint8_t number, ButtonConfig* config) {
    this->octave = octave;
    this->group = group;
    this->number = number;
    this->config = config;

    value = &config->value1[group][number];
  }

  virtual Color GetColor() { return config->color[group]; }
  virtual Dimension GetSize() { return Dimension(7, 2); }
  
  const uint8_t pianoNote[2][7] = {{ 0 , 1 , 3 , 0 , 6 , 8 , 10 },
                                    { 0 , 2 , 4 , 5 , 7 , 9 , 11}};


  virtual bool Render(Point origin) {

    for (uint8_t note = 0; note < 12; note++)
    {
      Point xy = origin + ((note < 5) ? Point((note + 1) / 2, (note + 1) % 2) : Point((note + 2) / 2, note % 2));

      if (note + *octave * 12 == *value) 
      { 
        MatrixOS::LED::SetColor(xy, Color(0xFFFFFF)); 
      }
      else if (note + *octave * 12 > 127)
      {
        MatrixOS::LED::SetColor(xy, Color(0x000000));
      }
      else
      {
        MatrixOS::LED::SetColor(xy, config->color[group]);
      }
    }
    return true;
  }
  
  

  
  
  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    if (xy == Point(0, 0) || xy == Point(3, 0))
      return false;

    uint8_t note = pianoNote[xy.y][xy.x] + *octave * 12;

    if (note > 127)
      return false; 

    if (keyInfo->state == PRESSED)  
    {
      MatrixOS::MIDI::Send(MidiPacket(0, NoteOn, config->channel[group][number], note, 127));
      *value = note;
    }
    else if (keyInfo->state == RELEASED)
    {
      MatrixOS::MIDI::Send(MidiPacket(0, NoteOff, config->channel[group][number], note, 0));
    }
    return true;
  }
  
};