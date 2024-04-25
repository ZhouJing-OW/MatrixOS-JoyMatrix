#pragma once
#include <cmath>
#include "../data/4pxNarrowfont.h"
#include "UIComponent.h"

class UI4pxNoteName : public UIComponent {
 public:
  Color color1;
  Color color2;
  Color color3;
  int8_t* note;

  const char noteName[12] = {'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B'};
  const uint16_t sharp = 0b010101001010;

  UI4pxNoteName(int8_t* note = nullptr, Color color1 = Color(YELLOW), Color color2 = Color(WHITE), Color color3 = Color(CYAN)) {
    this->color1 = color1;
    this->color2 = color2;
    this->color3 = color3;
    this->note = note;
  }

  virtual Dimension GetSize() { return Dimension(8, 4); }
  virtual Color GetColor() { return color1; };

  void Render4px(Point origin, Color color, char name) {
    for (int8_t x = 0; x < 3; x++)
    {
      for (int8_t y = 0; y < 4; y++)
      { MatrixOS::LED::SetColor(origin + Point(x, 3 - y), bitRead(narrowFont4[(int)name - 32][x + 1], y) ? color : Color(BLANK)); }
    }
  }

  void RenderSymbol(Point origin, Color color, bool isSharp, bool isNagative) 
  {
    for(int8_t x = 0; x < 2; x++)
    {
      for(int8_t y = 0; y < 2; y++)
        MatrixOS::LED::SetColor(origin + Point(x, y), isSharp ? color : Color(BLANK));
      MatrixOS::LED::SetColor(origin + Point(x, 2), Color(BLANK));
      MatrixOS::LED::SetColor(origin + Point(x, 3), isNagative ? color : Color(BLANK));
    }
  }

  virtual bool Render(Point origin) {
    if(note != nullptr && *note >= 0)
    {
      char name = noteName[*note % 12];
      int8_t octave = *note / 12;
      octave -= 2;
      bool isSharp = bitRead(sharp, *note % 12);
      Render4px(origin, color1, name);
      RenderSymbol(origin + Point(3, 0), color2, isSharp, octave < 0);
      Render4px(origin + Point(5, 0), color3, (char)(abs(octave) + 48));
    }
    return true;
  }
};