#include "MultiPad.h"

bool MultiPad::NoteRender(Point origin)
{
  uint8_t index = 0;
  for (int8_t y = 0; y < dimension.y; y++)
  {
    for (int8_t x = 0; x < dimension.x - 1; x++)
    {
      if(!(settingMode && (x >= padSettingArea.x || y >= padSettingArea.y)))
      {
        uint8_t note = noteMap[index];
        Point xy = origin + Point(x, y);

        Color thisColor, thisRootColor;
        switch(MatrixOS::MidiCenter::GetPadCheck(note))
        {
          case IN_NONE:   thisColor = COLOR_NOTE_PAD[0];    thisRootColor = COLOR_NOTE_PAD[1];    break;
          case IN_INPUT:  thisColor = Color(WHITE);         thisRootColor = Color(WHITE);         break;
          case IN_SEQ:    thisColor = Color(LAWN_LS);       thisRootColor = Color(LAWN);          break;
          case IN_ARP:    thisColor = Color(GOLD_LS);       thisRootColor = Color(GOLD_LS);       break;
          case IN_CHORD:  thisColor = Color(YELLOW_LS);     thisRootColor = Color(YELLOW);        break;
          case IN_VOICE:  thisColor = Color(VIOLET_LS);     thisRootColor = Color(VIOLET);        break;
        }

        if (note == 255)
          MatrixOS::LED::SetColor(xy, Color(BLANK));
        else
        {
          uint8_t inScale = InScale(note);  // Check if the note is in scale.
          if (inScale == 0)
            MatrixOS::LED::SetColor(xy, thisColor.Scale(64).Blink_Key(Device::KeyPad::fnState)); 
          else if (inScale == 1)
            MatrixOS::LED::SetColor(xy, thisColor.Blink_Key(Device::KeyPad::fnState)); 
          else if (inScale == 2)
            MatrixOS::LED::SetColor(xy, thisRootColor.Blink_Key(Device::KeyPad::fnState));
        }
      }
      index++;
    }
  }
  return true;
}

bool MultiPad::NoteKeyEvent(Point xy, KeyInfo* keyInfo)
{
  Point ui = xy - Point(1, 0);
  uint16_t pos = channelConfig->activePadConfig[channel][NOTE_PAD];
  uint8_t note = noteMap[ui.y * (dimension.x - 1) + ui.x];
  if (note == 255)
  {
    return false; }
    if (keyInfo->state == PRESSED) {
      if(Device::KeyPad::fnState.active()) {
        MatrixOS::Component::Pad_Setting(padConfig, pos, NOTE_PAD);
        GenerateNoteMap();
        return true;
      }
      channelConfig->activeNote[channel] = note;
      if(Device::KeyPad::Shift()) MatrixOS::MidiCenter::Toggle(SEND_NOTE, channel, note);
      else MatrixOS::MidiCenter::Hold(xy + position, SEND_NOTE, channel, note);
    }
    return true;
}

uint8_t MultiPad::InScale(uint8_t note) {
  note %= 12;
  uint16_t pos = channelConfig->activePadConfig[channel][NOTE_PAD];
  NotePadConfig* config = padConfig + pos;
  if (note == config->rootKey)
    return 2;  // It is a root key

  c_aligned_scale_map = ((config->scale << config->rootKey) + ((config->scale & 0xFFF) >> (12 - config->rootKey % 12))) & 0xFFF;  // Rootkey should  always < 12
  return bitRead(c_aligned_scale_map, note);
}

void MultiPad::GenerateNoteMap() {
  if (*padType == NOTE_PAD) 
  {
    Dimension noteMapArea = Dimension(dimension.x - 1, dimension.y);
    noteMap.reserve(noteMapArea.Area());
    NotePadConfig* config = padConfig + channelConfig->activePadConfig[channel][NOTE_PAD];
    uint8_t root = 12 * channelConfig->octave[channel] + config->rootKey;
    uint8_t nextNote = root;
    uint8_t rootCount = 0;
    for (int8_t y = 0; y < noteMapArea.y; y++)
    {
      int8_t ui_y = noteMapArea.y - y - 1;
      if(config->overlap && config->overlap < noteMapArea.x)
      { 
        if(y != 0) nextNote = noteMap[(ui_y + 1) * noteMapArea.x + config->overlap]; 
      }
      else if (config->alignRoot && rootCount >= 2)
      {
        root += 12;
        rootCount = 0;
        nextNote = root;
      }
      for (int8_t x = 0; x < noteMapArea.x; x++)
      {
        uint8_t id = ui_y * noteMapArea.x + x;
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
}
