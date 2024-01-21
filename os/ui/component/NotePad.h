#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class NotePad : public UIComponent {
 public:
  Dimension dimension;
  NotePadConfig* config;
  std::vector<uint8_t> noteMap;
  Point position = Point(0, 0);
  bool octaveView;
  uint32_t octaveTimer;
  int8_t lastOctave;

  NotePad(Dimension dimension, NotePadConfig* config) {
    this->dimension = dimension;
    this->config = config;
    this->lastOctave = config->octave;
    octaveView = false;
    GenerateKeymap();
  }

  virtual Color GetColor() { return config->color; }
  virtual Dimension GetSize() { return dimension; }

  uint8_t InScale(uint8_t note) {
    note %= 12;

    if (note == config->rootKey)
      return 2;  // It is a root key

    uint16_t c_aligned_scale_map = ((config->scale << config->rootKey) + ((config->scale & 0xFFF) >> (12 - config->rootKey % 12))) & 0xFFF;  // Rootkey should  always < 12
    return bitRead(c_aligned_scale_map, note);
  }

  void GenerateKeymap() {
    noteMap.reserve(dimension.Area());

    uint8_t root = 12 * config->octave + config->rootKey;
    uint8_t nextNote = root;
    uint8_t rootCount = 0;
    for (int8_t y = 0; y < dimension.y; y++)
    {
      int8_t ui_y = dimension.y - y - 1;
      if(config->overlap && config->overlap < dimension.x)
      { 
        if(y != 0) nextNote = noteMap[(ui_y + 1) * dimension.x + config->overlap]; 
      }
      else if (config->alignRoot && rootCount >= 2)
      {
        root += 12;
        rootCount = 0;
        nextNote = root;
      }
      for (int8_t x = 0; x < dimension.x; x++)
      {
        uint8_t id = ui_y * dimension.x + x;
        if (nextNote > 127)
        {
          noteMap[id] = 255;
          continue;
        }
        if(!config->enfourceScale)
        {
          noteMap[id] = nextNote;  // Add to map
          nextNote++;
          continue;
        }
        while (true)  // Find next key that we should put in
        {
          uint8_t inScale = InScale(nextNote);
          if (inScale == 2)
          { rootCount++; }  // If root detected, inc rootCount
          if (inScale)      // If is in scale
          {
            noteMap[id] = nextNote;  // Add to map
            nextNote++;
            break;  // Break from inf loop
          }
          else
          {
            nextNote++;
            continue;  // Check next note
          }
        }
      }
    }
  }

  virtual bool Render(Point origin) {
    position = origin;

    if (dimension.x > 9){
      if (lastOctave != config->octave) {
        octaveTimer = MatrixOS::SYS::Millis() + 200; 
        lastOctave = config->octave;
        octaveView = true;
      }
      if (octaveTimer < MatrixOS::SYS::Millis()) {
        octaveView = false; octaveTimer = 0; 
      }
    }

    if(octaveView) {
        uint16_t octaveX = (dimension.x) / 2 - 4;
        for (int8_t y = 0; y < dimension.y; y++)
        {
          for (int8_t x = 0; x < dimension.x; x++)
          {
            Point xy = origin + Point(x, y);
            if (x >= octaveX && x < octaveX + 9 && y == dimension.y - 1) {
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

    if(!octaveView) {
      uint8_t index = 0;
      for (int8_t y = 0; y < dimension.y; y++)
      {
        for (int8_t x = 0; x < dimension.x; x++)
        {
          uint8_t note = noteMap[index];
          Point globalPos = origin + Point(x, y);
          uint8_t channel = config->globalChannel ? MatrixOS::UserVar::global_MIDI_CH : config->channel;

          if (note == 255)
          { MatrixOS::LED::SetColor(globalPos, COLOR_BLANK); }
          else if (MatrixOS::MIDI::CheckHold(SEND_NOTE, channel, note))  // If find the note is currently active. Show it as white
          { MatrixOS::LED::SetColor(globalPos, COLOR_WHITE); }
          else
          {
            uint8_t inScale = InScale(note);  // Check if the note is in scale.
            if (inScale == 0)
            { MatrixOS::LED::SetColor(globalPos, config->color.ToLowBrightness().Blink(Device::KeyPad::fnState)); }
            else if (inScale == 1)
            { MatrixOS::LED::SetColor(globalPos, config->color.Blink(Device::KeyPad::fnState)); }
            else if (inScale == 2)
            { MatrixOS::LED::SetColor(globalPos, config->rootColor.Blink(Device::KeyPad::fnState)); }
          }
          index++;
        }
      }
    }
    return true;
  }

  
  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    uint8_t note = noteMap[xy.y * dimension.x + xy.x];
    uint8_t channel = config->globalChannel ? MatrixOS::UserVar::global_MIDI_CH : config->channel;

    if(!octaveView){
      if (note == 255)
      { return false; }
      if (keyInfo->state == PRESSED) {
        if(Device::KeyPad::fnState == ACTIVATED || Device::KeyPad::fnState == HOLD) {
          MatrixOS::Component::Pad_Setting(config);
          GenerateKeymap();
          return true;
        }
        MatrixOS::MIDI::Hold(xy + position, SEND_NOTE, channel, note);
      }
      return true;
    }
    return false;
  }
  
};