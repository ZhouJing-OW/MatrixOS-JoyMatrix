#pragma once

#include "MatrixOS.h"

class PianoPad : public UIComponent {
 public:
  int8_t octaveOffset;
  MainPadConfig* config;
  // std::vector<uint8_t> noteMap;
  std::unordered_map<uint8_t, uint8_t> activeNotes;
  uint8_t AT;

  PianoPad(int8_t octaveOffset, MainPadConfig* config) {
    this->octaveOffset = octaveOffset;
    this->config = config;
    activeNotes.reserve(8);
  }

  virtual Color GetColor() { return config->rootColor; }
  virtual Dimension GetSize() { return Dimension(7, 2); }
  
  const uint8_t pianoNote[2][7] = {{ 0 , 1 , 3 , 0 , 6 , 8 , 10 },
                                    { 0 , 2 , 4 , 5 , 7 , 9 , 11}};


  virtual bool Render(Point origin) {
    uint16_t c_aligned_scale_map = ((config->scale << config->rootKey) + ((config->scale & 0xFFF) >> (12 - config->rootKey % 12))) & 0xFFF;  // Rootkey should always < 12,
                                                                                                                                              // might add an assert later
    for (uint8_t note = 0; note < 12; note++)
    {
      Point xy = origin + ((note < 5) ? Point((note + 1) / 2, (note + 1) % 2) : Point((note + 2) / 2, note % 2));

      if (activeNotes.find(note + (config->octave + octaveOffset) * 12 ) != activeNotes.end())  // If find the note is currently active. Show it as white
        { MatrixOS::LED::SetColor(xy, Color(0xFFFFFF)); }

      else if (note + (config->octave + octaveOffset) * 12 > 127)
      {
        MatrixOS::LED::SetColor(xy, Color(0x000000));
      }
      else if (note == config->rootKey)
      {
        MatrixOS::LED::SetColor(xy, config->rootColor);
      }
      else if (bitRead(c_aligned_scale_map, note))
      {
        MatrixOS::LED::SetColor(xy, config->color);
      }
      else
      {
        MatrixOS::LED::SetColor(xy, config->color.ToLowBrightness());
      }
    }
    return true;
  }
  
  

  
  
  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    if (xy == Point(0, 0) || xy == Point(3, 0))
      return false;

    uint8_t note = pianoNote[xy.y][xy.x] + (config->octave + octaveOffset) * 12;

    if (note > 127)
      return false; 

    if (keyInfo->state == PRESSED)  
    {
      uint8_t pressure;
      if (Device::pressure > MatrixOS::UserVar::pressureToVelocity_Min)
        pressure = Device::pressure;
      else
        pressure = MatrixOS::UserVar::pressureToVelocity_Min;
      if (pressure > MatrixOS::UserVar::pressureToVelocity_Max)
        pressure = MatrixOS::UserVar::pressureToVelocity_Max;

      MatrixOS::MIDI::Send(MidiPacket(0, NoteOn, config->channel, note, MatrixOS::UserVar::pressureSensitive ? pressure : MatrixOS::UserVar::pressureToVelocity_Default));
      activeNotes[note]++;  // If this key doesn't exist, unordered_map will auto assign it to 0.
    }
    else if (MatrixOS::UserVar::pressureSensitive && keyInfo->state == AFTERTOUCH)
    { 
      if (Device::pressure != AT){
        MatrixOS::MIDI::Send(MidiPacket(0, AfterTouch, config->channel, note, Device::pressure));
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, config->channel, 74, Device::pressure));
        AT = Device::pressure;
      }
    }
    else if (keyInfo->state == RELEASED)
    {
      MatrixOS::MIDI::Send(MidiPacket(0, NoteOff, config->channel, note, 0));
      if (activeNotes[note]-- <= 1)
      { activeNotes.erase(note); }
    }
    return true;
  }
  
};