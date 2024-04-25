#include "MultiPad.h"

bool MultiPad::DrumRender(Point origin)
{
  uint8_t width = 8 ; // 16 / dimension.y + (16 % dimension.y > 0)
  uint16_t pos = channelConfig->activePadConfig[channel][DRUM_PAD] * 16;
  MidiButtonConfig* con = drumConfig + pos;
  for (uint8_t x = 0; x < dimension.x; x++)
  {
    for (uint8_t y = dimension.y - 2; y < dimension.y; y++)
    {
      Point xy = origin + Point(x, y);
      if (x < width)
      {
        uint8_t i = x + (dimension.y - 1 - y) * width;
        if (i < 16)
        {
          if (MatrixOS::MidiCenter::FindHold((con + i)->type, channel, (con + i)->byte1))
            MatrixOS::LED::SetColor(xy, Color(WHITE));
          else 
            MatrixOS::LED::SetColor(xy, (con + i)->color.Blink_Key(Device::KeyPad::fnState));
        }
        else
          MatrixOS::LED::SetColor(xy, Color(BLANK));
      }
      else if (x >= dimension.x - width && !settingMode)
      {
        uint8_t i = x - (dimension.x - width) + (dimension.y - 1 - y) * width;
        if (i < 16){
          if (MatrixOS::MidiCenter::FindHold((con + i)->type, channel, (con + i)->byte1))
            MatrixOS::LED::SetColor(xy, Color(WHITE));
          else if ((con + i)->byte1 == channelConfig->activeNote[channel])
            MatrixOS::LED::SetColor(xy, (con + i)->color.Blink_Key(Device::KeyPad::fnState));
          else
            MatrixOS::LED::SetColor(xy, (con + i)->color.ToLowBrightness().Blink_Key(Device::KeyPad::fnState));
        }
        else
          MatrixOS::LED::SetColor(xy, Color(BLANK));
      }
      else if (!(settingMode && x >= dimension.x - 5))
        MatrixOS::LED::SetColor(xy, Color(BLANK));
    }
  }

  if (dimension.y == 2) return true;

  for (uint8_t x = 1; x < 16; x++) {
    for (uint8_t y = 0; y < 2; y++) {
      MatrixOS::LED::SetColor(origin + Point(x, y), Color(BLANK));
    }
  }
 
  return true;
}

bool MultiPad::DrumKeyEvent(Point xy, KeyInfo* keyInfo)
{
  if (keyInfo->state == PRESSED)
  {
    if(dimension.y == 4 && xy.y < 2) return false;
    
    uint8_t width = 8; // 16 / dimension.y + (16 % dimension.y > 0);
    uint16_t pos = channelConfig->activePadConfig[channel][DRUM_PAD] * 16;
    MidiButtonConfig* con = drumConfig + pos;
    uint8_t i;
    if (xy.x < width)
    {
      i = xy.x + (dimension.y - 1 - xy.y) * width;
      if((Device::KeyPad::fnState.active())) 
      {
        channelConfig->activeNote[channel] = (con + i)->byte1;
        MatrixOS::Component::DrumNote_Setting(drumConfig, pos + i);
        return true;
      } 
    }
    else if (xy.x >= dimension.x - width && !settingMode)
      i = xy.x - (dimension.x - width) + (dimension.y - 1 - xy.y) * width;
    else
      return false;
    channelConfig->activeNote[channel] = (con + i)->byte1;
    MatrixOS::MidiCenter::Hold(xy + position, (con + i)->type, channel, (con + i)->byte1, (con + i)->byte2);
    // noteNameView++;
    return true;
  }
  
  if (keyInfo->state == RELEASED)
  {
    // noteNameView--;
  }
  
  return false;
}



