#include "MultiPad.h"

bool MultiPad::OctaveRender(Point origin)
{
  NotePadConfig* config = padConfig + channelConfig->activePadConfig[channel][*padType];
  Color color = (*padType == PIANO_PAD) ? COLOR_PIANO_PAD[0] : COLOR_NOTE_PAD[0];
  uint16_t octaveX = (dimension.x - 1) / 2 - 5;
  for (int8_t y = 0; y < dimension.y; y++)
  {
    for (int8_t x = 0; x < dimension.x - 1; x++)
    {
      Point xy = origin + Point(x, y);
      if (x >= octaveX && x < octaveX + 10 && y == dimension.y - 1) {
        if (x == octaveX + config->octave) {
          MatrixOS::LED::SetColor(xy, Color(WHITE));
        } else {
          MatrixOS::LED::SetColor(xy, color.ToLowBrightness());
        }
      } else {
        MatrixOS::LED::SetColor(xy, Color(BLANK));
      }
    }
  }
  return true;
}

void MultiPad::OctaveShiftRender(Point origin)
{ 
  Color color = Color(WHITE);
  Color color1 = color.Scale((int8_t)((float)padConfig->octave * 239/ 9 + 16));
  Color color2 = color.Scale((int8_t)((1 - (float)padConfig->octave) * 239 / 9 + 16));
  for(uint8_t y = 0; y < dimension.y; y++)
  {
    Point xy = origin + Point(0, y);
    if (y == dimension.y - 2 ) MatrixOS::LED::SetColor(xy, color1);
    else if (y == dimension.y - 1 ) MatrixOS::LED::SetColor(xy, color2);
  }

}

bool MultiPad::OctaveShiftKeyEvent(Point xy, KeyInfo* keyInfo)
{
    if (keyInfo->state == PRESSED && xy.y >= dimension.y - 2)
    { 
      NotePadConfig* config = padConfig + channelConfig->activePadConfig[channel][*padType];
      int8_t *octave = &config->octave;
      uint8_t i = xy.y - (dimension.y - 2);
      if (i == 0) *octave = (*octave + 1) < 9 ? (*octave + 1) : 9;
      if (i == 1) *octave = (*octave - 1) > 0 ? (*octave - 1) : 0;
      octaveTimer.RecordCurrent();
      octaveViewMode = true;
      return true;
    }
    return false;
  return false;
}