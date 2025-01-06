#pragma once
#include "MatrixOS.h"
#include "MidiCenter.h"
#include "NodeUI.h"
#include <map>

namespace MatrixOS::MidiCenter
{
  const Color palette[128] = {
    // LaunchpadXcolorPalette (Legacy Palette)
    0x00000000,  // 0
    0x003F3F3F,  // 1
    0x007F7F7F,  // 2
    0x00FFFFFF,  // 3
    0x00FF3F3F,  // 4
    0x00FF0000,  // 5
    0x007F0000,  // 6
    0x003F0000,  // 7
    0x00FFBF6F,  // 8
    0x00FF3F00,  // 9
    0x007F1F00,  // 10
    0x003F0F00,  // 11
    0x00FFAF2F,  // 12
    0x00FFFF00,  // 13
    0x007F7F00,  // 14
    0x003F3F00,  // 15
    0x007FFF2F,  // 16
    0x004FFF00,  // 17
    0x002F7F00,  // 18
    0x00173F00,  // 19
    0x004FFF3F,  // 20
    0x0000FF00,  // 21
    0x00007F00,  // 22
    0x00003F00,  // 23
    0x004FFF4F,  // 24
    0x0000FF1F,  // 25
    0x00007F0F,  // 26
    0x00003F07,  // 27
    0x004FFF5F,  // 28
    0x0000FF5F,  // 29
    0x00007F2F,  // 30
    0x00003F17,  // 31
    0x004FFFBF,  // 32
    0x0000FF9F,  // 33
    0x00007F4F,  // 34
    0x00003F27,  // 35
    0x004FBFFF,  // 36
    0x0000AFFF,  // 37
    0x0000577F,  // 38
    0x00002F3F,  // 39
    0x004F7FFF,  // 40
    0x000057FF,  // 41
    0x00002F7F,  // 42
    0x0000173F,  // 43
    0x002F1FFF,  // 44
    0x000000FF,  // 45
    0x0000007F,  // 46
    0x0000003F,  // 47
    0x005F3FFF,  // 48
    0x002F00FF,  // 49
    0x0017007F,  // 50
    0x000F003F,  // 51
    0x00FF3FFF,  // 52
    0x00FF00FF,  // 53
    0x007F007F,  // 54
    0x003F003F,  // 55
    0x00FF3F6F,  // 56
    0x00FF004F,  // 57
    0x007F002F,  // 58
    0x003F001F,  // 59
    0x00FF0F00,  // 60
    0x009F3F00,  // 61
    0x007F4F00,  // 62
    0x002F2F00,  // 63
    0x00003F00,  // 64
    0x00003F1F,  // 65
    0x00001F6F,  // 66
    0x000000FF,  // 67
    0x00003F3F,  // 68
    0x001F00BF,  // 69
    0x005F3F4F,  // 70
    0x001F0F17,  // 71
    0x00FF0000,  // 72
    0x00BFFF2F,  // 73
    0x00AFEF00,  // 74
    0x005FFF00,  // 75
    0x000F7F00,  // 76
    0x0000FF5F,  // 77
    0x00009FFF,  // 78
    0x00002FFF,  // 79
    0x001F00FF,  // 80
    0x005F00EF,  // 81
    0x00AF1F7F,  // 82
    0x002F0F00,  // 83
    0x00FF2F00,  // 84
    0x007FDF00,  // 85
    0x006FFF1F,  // 86
    0x0000FF00,  // 87
    0x003FFF2F,  // 88
    0x005FEF6F,  // 89
    0x003FFFCF,  // 90
    0x005F8FFF,  // 91
    0x002F4FCF,  // 92
    0x006F4FDF,  // 93
    0x00DF1FFF,  // 94
    0x00FF005F,  // 95
    0x00FF4F00,  // 96
    0x00BFAF00,  // 97
    0x008FFF00,  // 98
    0x007F5F00,  // 99
    0x003F2F00,  // 100
    0x0000470F,  // 101
    0x000F4F1F,  // 102
    0x0017172F,  // 103
    0x00171F5F,  // 104
    0x005F3717,  // 105
    0x007F0000,  // 106
    0x00DF3F2F,  // 107
    0x00DF470F,  // 108
    0x00FFBF1F,  // 109
    0x009FDF2F,  // 110
    0x006FAF0F,  // 111
    0x0017172F,  // 112
    0x00DFDF6F,  // 113
    0x007FEF8F,  // 114
    0x009F9FFF,  // 115
    0x008F6FFF,  // 116
    0x003F3F3F,  // 117
    0x006F6F6F,  // 118
    0x00DFFFFF,  // 119
    0x009F0000,  // 120
    0x00370000,  // 121
    0x0017CF00,  // 122
    0x00003F00,  // 123
    0x00BFAF00,  // 124
    0x003F2F00,  // 125
    0x00AF4F00,  // 126
    0x004F0F00   // 127
  };

  class FeedBackUI : public NodeUI
  {
  public:

    std::unordered_set<uint16_t> actived;
    bool shift = false;
    int8_t shiftScreen = 0;

    FeedBackUI () {}

    ~FeedBackUI() 
    {
      Device::AnalogInput::DisableUpDown();

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
      position = origin;
      Device::AnalogInput::SetUpDown(&fullScreen, 1, -1);

      if(Device::KeyPad::Shift() && fullScreen == 0) {
        fullScreen = 1;
        shiftScreen = 1;
      }

      if(!Device::KeyPad::Shift() && shiftScreen == 1) {
        fullScreen = 0;
        shiftScreen = 0;
      }

      if(!fullScreen) dimension = Dimension(16, 2);
      else dimension = Dimension(16, 4);

      if (shift != Device::KeyPad::Shift())
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
          MatrixOS::LED::SetColor(origin + Point(x, y), Color(palette[FBButtons[n + shift * 64].second]));
        }
      }
        return true;
    }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo)
    {
      uint8_t n = xy.x + xy.y * dimension.x;
      
      uint16_t midiID = FBButtons[n + shift * 64].first;
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

    void SetNull(){}
    bool SetKnobPtr() { return false; }

   private:
    Point position = Point(0, 0);
    int8_t upDown = 0;

  };

}