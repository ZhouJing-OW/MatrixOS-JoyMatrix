#include "MultiPad.h"

bool MultiPad::DrumRender(Point origin)
{
  uint8_t width = 16 / dimension.y + (16 % dimension.y > 0);
  uint16_t pos = channelConfig->activePadConfig[channel][DRUM_PAD] * 16;
  MidiButtonConfig* con = drumConfig + pos;

  for (uint8_t x = 0; x < dimension.x; x++)
  {
    for (uint8_t y = 0; y < dimension.y; y++)
    {
      Point xy = origin + Point(x, y);
      uint8_t i;

      if (x < width)
      {
        i = x + y * width;
        if (i < 16)
        {
          Color thisColor = GetPadColor((con + i)->byte1);
          thisColor = thisColor != Color(BLANK) ? thisColor : (con + i)->color;
          MatrixOS::LED::SetColor(xy, thisColor.Blink_Key(Device::KeyPad::fnState));
        }
        else
          MatrixOS::LED::SetColor(xy, Color(BLANK));
      }
      else if (x >= dimension.x - width && !settingMode)
      {
        i = x - (dimension.x - width) + y * width;
        if (i < 16){
          Color thisColor = GetPadColor((con + i)->byte1);
          thisColor = thisColor != Color(BLANK) ? thisColor : (con + i)->color;
          if ((con + i)->byte1 == channelConfig->activeNote[channel])
            MatrixOS::LED::SetColor(xy, thisColor.Blink_Key(Device::KeyPad::fnState));
          else
            MatrixOS::LED::SetColor(xy, thisColor.Scale(64).Blink_Key(Device::KeyPad::fnState));
        }
        else
          MatrixOS::LED::SetColor(xy, Color(BLANK));
      }
      else if(x >= width && x < dimension.x - width && !settingMode)
      {
        uint8_t rate = xy.x - width + 1;
        uint8_t velocity = 128 / dimension.y * (dimension.y - y) - 1;
        uint8_t trig = MatrixOS::MidiCenter::GetRetrigCheck(rate, velocity);
        switch (trig)
        {
          case 0: MatrixOS::LED::SetColor(xy, Color(RED).Dim(velocity)); break;
          case 1: MatrixOS::LED::SetColor(xy, Color(RED_HL).Dim(velocity)); break;
          case 2: MatrixOS::LED::SetColor(xy, Color(WHITE).Dim(velocity)); break;
        }
      }
      else if (!(settingMode && x >= dimension.x - 5))
        MatrixOS::LED::SetColor(xy, Color(BLANK));
    }
  }

  return true;
}

Color MultiPad::GetPadColor(uint8_t note)
{
  switch(MatrixOS::MidiCenter::GetPadCheck(note))
  {
    case IN_NONE:   return Color(BLANK);   
    case IN_INPUT:  return Color(WHITE);         
    case IN_SEQ:    return Color(LAWN_LS);       
    case IN_ARP:    return Color(BLANK);       
    case IN_CHORD:  return Color(BLANK);     
    case IN_VOICE:  return Color(BLANK);      
  }
  return Color(BLANK);
}

bool MultiPad::DrumKeyEvent(Point xy, KeyInfo* keyInfo)
{
  if (keyInfo->state == PRESSED)
  {
    uint8_t width = 16 / dimension.y + (16 % dimension.y > 0);
    uint16_t pos = channelConfig->activePadConfig[channel][DRUM_PAD] * 16;
    MidiButtonConfig* con = drumConfig + pos;
    uint8_t i = 0;
    if (xy.x < width)
    {
      i = xy.x + xy.y * width;
      if((Device::KeyPad::fnState.active())) 
      {
        channelConfig->activeNote[channel] = (con + i)->byte1;
        MatrixOS::Component::DrumNote_Setting(drumConfig, pos + i);
        return true;
      } 
    }
    else if (xy.x >= dimension.x - width && !settingMode)
      i = xy.x - (dimension.x - width) + xy.y * width;
    else if (xy.x >= width && xy.x < dimension.x - width && !settingMode)
    {
      uint8_t rate = xy.x - width + 1;
      uint8_t velocity = 128 / dimension.y * (dimension.y - xy.y) - 1;
      MatrixOS::MidiCenter::Retrig(xy, rate, channel, channelConfig->activeNote[channel] , velocity);
      return true;
    }

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



