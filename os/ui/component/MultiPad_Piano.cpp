#include "MultiPad.h"

bool MultiPad::PianoRender(Point origin)
{
  uint16_t pos = channelConfig->activePadConfig[channel][PIANO_PAD];
  NotePadConfig* config = padConfig + pos;

  Color color = COLOR_PIANO_PAD[0];
  Color rootColor = COLOR_PIANO_PAD[1];
  uint8_t n = 0;
  for(int8_t x = 0; x < dimension.x - 1; x++){
    for(int8_t y = 0; y < dimension.y; y++){
      if(settingMode && (x >= padSettingArea.x || y >= padSettingArea.y))
        continue;

      Point xy = origin + Point(x, y);
      if(pianoMap[n] >= 0)
      {
        if (MatrixOS::MidiCenter::FindArp(SEND_NOTE, channel, pianoMap[n]))
          MatrixOS::LED::SetColor(xy, COLOR_ORANGE);
        else if (MatrixOS::MidiCenter::FindHold(SEND_NOTE, channel, pianoMap[n])) // If find the note is currently active. Show it as white
          MatrixOS::LED::SetColor(xy, COLOR_WHITE); 
        else if (pianoMap[n] % 12 == config->rootKey)
          MatrixOS::LED::SetColor(xy, rootColor.Blink_Key(Device::KeyPad::fnState));
        else if (bitRead(c_aligned_scale_map, pianoMap[n] % 12))
          MatrixOS::LED::SetColor(xy, color.Blink_Key(Device::KeyPad::fnState));
        else 
          MatrixOS::LED::SetColor(xy, color.ToLowBrightness().Blink_Key(Device::KeyPad::fnState));
      }
      else 
        MatrixOS::LED::SetColor(xy, COLOR_BLANK);
      
      n++;
    }
  }
  return true;
}

bool MultiPad::PianoKeyEvent(Point xy, KeyInfo* keyInfo)
{
  Point ui = xy - Point(1, 0);
  uint8_t n = ui.x * (dimension.y) + ui.y;
  
  if (pianoMap[n] < 0)
    return false; 
  else if (keyInfo->state == PRESSED) {
    if((Device::KeyPad::fnState.active())) 
    {
      uint16_t pos = channelConfig->activePadConfig[channel][PIANO_PAD];
      MatrixOS::Component::Pad_Setting(padConfig, pos, PIANO_PAD);
      GeneratePianoMap();
      return true;
    }
    if(Device::KeyPad::Shift()) MatrixOS::MidiCenter::Toggle(SEND_NOTE, channel, pianoMap[n]);
    else MatrixOS::MidiCenter::Hold(xy + position, SEND_NOTE, channel, pianoMap[n]);
  } 
  return true;

}

void MultiPad::GeneratePianoMap()
{
  if (*padType == PIANO_PAD)
  {
    uint8_t pos = channelConfig->activePadConfig[channel][PIANO_PAD];
    NotePadConfig* config = padConfig + pos;
    c_aligned_scale_map = ((config->scale << config->rootKey) + ((config->scale & 0xFFF) >> (12 - config->rootKey % 12))) & 0xFFF;

    pianoMap.clear();
    pianoMap.reserve(Dimension(dimension.x - 1, dimension.y).Area());
    
    for(int8_t x = 0; x < dimension.x - 1; x++){
      for(int8_t y = 0; y < dimension.y; y++){
        {
          int8_t octaveshift = x - config->shift < 0 ? -1 : ((x - config->shift) / 7);
          int8_t octave = config->octave + octaveshift + ((dimension.y - y - 1) / 2) * ((dimension.x - 1) / 7);
          int8_t note;
          if (pianoNote[(dimension.y + y) % 2][(x - config->shift + 7) % 7] == -1)
            note = -1;
          else
            note = pianoNote[(dimension.y + y) % 2][(x - config->shift + 7) % 7] + octave * 12;
          pianoMap.push_back(note);
        }
      }
    }
  }
}

