#pragma once

#include "UIComponent.h"
#include "MatrixOS.h"

class NotePreView : public UIComponent {
 public:
  Dimension dimension = Dimension(15, 2);
  Dimension view;
  NotePadConfig* config;
  std::vector<uint8_t> noteMap;
  Point position = Point(0, 0);
  Color color = COLOR_NOTE_PAD[0];
  Color rootColor = COLOR_NOTE_PAD[1];
  uint16_t scalePrv;
  bool alignRootPrv;
  bool enfourceScalePrv;
  int8_t rootKeyPrv;

  NotePreView(Dimension view, NotePadConfig* config) {
    this->view = view;
    this->config = config;
    GenerateKeymap();  }

  virtual Color GetColor() { return rootColor; }
  virtual Dimension GetSize() { return view; }

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
          uint8_t inScale = InScale(nextNote);
          if (inScale == 2)
          { rootCount++; } 
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

    if(scalePrv != config->scale || enfourceScalePrv != config->enfourceScale || rootKeyPrv != config->rootKey || alignRootPrv != config->alignRoot) 
    {
      GenerateKeymap();
      scalePrv = config->scale;
      enfourceScalePrv = config->enfourceScale;
      rootKeyPrv = config->rootKey;
      alignRootPrv = config->alignRoot;
    }

    uint8_t index = 0;
    for (int8_t y = 0; y < dimension.y; y++)
    {
      for (int8_t x = 0; x < dimension.x; x++)
      {
        uint8_t note = noteMap[index];
        Point globalPos = origin + Point(x, y);
        uint8_t channel = config->globalChannel ? MatrixOS::UserVar::global_channel : config->channel;

        if (note == 255 && x < view.x && y < view.y)
          MatrixOS::LED::SetColor(globalPos, Color(BLANK)); 
        else if (y == dimension.y - 1 && x == config->overlap)
          MatrixOS::LED::SetColor(globalPos, Color(YELLOW));
        else if (MatrixOS::MidiCenter::FindHold(SEND_NOTE, channel, note))  // If find the note is currently active. Show it as white
          MatrixOS::LED::SetColor(globalPos, Color(WHITE));
        else
        {
          uint8_t inScale = InScale(note);  // Check if the note is in scale.
          if (inScale == 0)
          { 
            MatrixOS::LED::SetColor(globalPos, color.ToLowBrightness()); 
          }
          else if (inScale == 1)
            MatrixOS::LED::SetColor(globalPos, color); 
          else if (inScale == 2)
            MatrixOS::LED::SetColor(globalPos, rootColor);
        }
        index++;
      }
    }
  
    return true;
  }

  
  virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    uint8_t note = noteMap[xy.y * dimension.x + xy.x];
    uint8_t channel = config->globalChannel ? MatrixOS::UserVar::global_channel : config->channel;
      if (note == 255)
        return false; 
      if (keyInfo->state == PRESSED)
      {
        if (xy.y == dimension.y - 1 && xy.x != config->overlap)
        {
          config->overlap = xy.x;
          GenerateKeymap();
        }
        MatrixOS::MidiCenter::Hold(xy + position, SEND_NOTE, channel, note);
        return true;
      }
      if (keyInfo->state == HOLD){ return true; }
      
    return false;
  }
  
};