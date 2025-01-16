#pragma once
#include "MatrixOS.h"
#include "MidiCenter.h"
#include <map>

namespace MatrixOS::MidiCenter
{
  extern const Color palette[128];

  class FeedBackButtons : public UIComponent
  {
  public:
    Dimension dimension = Dimension(4, 1);
    std::unordered_set<uint16_t> actived;
    pair<uint16_t, uint8_t>* buttons;
    uint8_t count = 0;
    bool shift = false;

    FeedBackButtons (Dimension dimension, pair<uint16_t, uint8_t>* buttons, uint8_t count) 
    {
      this->dimension = dimension;
      this->buttons = buttons;
      this->count = count;
    }

    ~FeedBackButtons() 
    {
      for (auto it = actived.begin(); it != actived.end(); it++)
      {
        MatrixOS::MidiCenter::Send_Off(ID_Type(*it), ID_Channel(*it), ID_Byte1(*it));
      }
      actived.clear();
    }

    virtual Color GetColor() { return Color(GREEN); }
    virtual Dimension GetSize() { return dimension; }

    virtual bool Render(Point origin)
    {
      if (dimension.Area() < count && shift != Device::KeyPad::Shift())
      {
        for (auto it = actived.begin(); it != actived.end(); it++)
        {
          MatrixOS::MidiCenter::Send_Off(ID_Type(*it), ID_Channel(*it), ID_Byte1(*it));
        }
        actived.clear();
        shift = Device::KeyPad::Shift();
      }
      
      for (int x = 0; x < dimension.x; x++)
      {
        for (int y = 0; y < dimension.y; y++)
        {
          uint8_t n = x + y * dimension.x;
          MatrixOS::LED::SetColor(origin + Point(x, y), Color(palette[buttons[n + shift * count / 2].second]));
        }
      }
        return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
    {
      uint8_t n = xy.x + xy.y * dimension.x;
      
      uint16_t midiID = buttons[n + shift * count / 2].first;
      if (keyInfo->state == PRESSED){
        MatrixOS::MidiCenter::Send_On(ID_Type(midiID), ID_Channel(midiID), ID_Byte1(midiID), 127);
        actived.insert(midiID);
        return true;
      }

      if (keyInfo->state == RELEASED){
        MatrixOS::MidiCenter::Send_Off(ID_Type(midiID), ID_Channel(midiID), ID_Byte1(midiID));
        actived.erase(midiID);
        return true;
      }
      return false;
    }
  };

}