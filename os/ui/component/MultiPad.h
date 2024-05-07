#pragma once


#include "MatrixOS.h"
#include "system/MIDIAPP/MidiCenter.h"
#include "UIComponent.h"
#include "UI4pxNoteName.h"

class MultiPad : public UIComponent {
 public:
  Dimension dimension;
  Dimension dimensionPrv;
  uint8_t configCount;
  ChannelConfig* channelConfig;
  NotePadConfig* padConfig;
  MidiButtonConfig* drumConfig;
  // UI4pxNoteName noteName;

  Dimension padSettingArea;
  Dimension drumSettingArea;

  std::vector<uint8_t> noteMap;
  std::vector<int8_t> pianoMap;
  uint16_t c_aligned_scale_map;
  Point position = Point(0, 0);
  bool octaveViewMode = false;
  bool settingMode = false;
  // uint8_t noteNameView = false;
  Timer octaveTimer;
  uint8_t channel = MatrixOS::UserVar::global_channel;
  uint8_t channelPrv;
  int8_t* padType;

  // Piano Pad
  bool PianoRender(Point origin);
  bool PianoKeyEvent(Point xy, KeyInfo* keyInfo);
  void GeneratePianoMap();
  // Note Pad
  bool NoteRender(Point origin);
  bool NoteKeyEvent(Point xy, KeyInfo* keyInfo);
  uint8_t InScale(uint8_t note);
  void GenerateNoteMap();
  // Drum Pad
  bool DrumRender(Point origin);
  bool DrumKeyEvent(Point xy, KeyInfo* keyInfo);
  Color GetPadColor(uint8_t note);
  // octave
  bool OctaveRender(Point origin);
  void OctaveShiftRender(Point origin);
  bool OctaveShiftKeyEvent(Point xy, KeyInfo* keyInfo);
  // Setting
  bool SettingRender(Point origin);
  bool SettingKeyEvent(Point xy, KeyInfo* keyInfo);

  MultiPad(Dimension dimension, uint8_t configCount, ChannelConfig* channelConfig, NotePadConfig* padConfig, MidiButtonConfig* drumConfig = nullptr) {
    this->dimension = dimension;
    this->channelConfig = channelConfig;
    this->padConfig = padConfig;
    this->drumConfig = drumConfig;
    this->configCount = configCount;
    channel = MatrixOS::UserVar::global_channel;
    channelPrv = channel;
    dimensionPrv = dimension;
    padType = &channelConfig->padType[channel];
    GenerateNoteMap();
    GeneratePianoMap();
    padSettingArea = Dimension(dimension.x - 6, dimension.y);
    drumSettingArea = Dimension(dimension.x - 5, dimension.y);
  }

  virtual Color GetColor() { return Color(WHITE); }
  virtual Dimension GetSize() { return dimension; }

  const int8_t pianoNote[2][7] = {{ -1 , 1 , 3 , -1 , 6 , 8 , 10 },
                                  {  0 , 2 , 4 ,  5 , 7 , 9 , 11 }};

  virtual bool Render(Point origin) {
    position = origin;
    channel = MatrixOS::UserVar::global_channel;
    padType = &channelConfig->padType[channel];

    if (channel != channelPrv || dimension != dimensionPrv)
    {
      GenerateNoteMap();
      GeneratePianoMap();
      channelPrv = channel;
      dimensionPrv = dimension;
    }

    if(octaveViewMode && octaveTimer.IsLonger(400))
    {
      octaveViewMode = false;
      GenerateNoteMap();
      GeneratePianoMap();
    }

    if(octaveViewMode)
      return OctaveRender(origin + Point(1,0));

    if (Device::KeyPad::fnState.active())
      settingMode = true;
    else
      settingMode = false;

    if(settingMode)
      SettingAreaRender(origin + Point(1, 0));

    switch (*padType)
    {
      case NOTE_PAD:
        OctaveShiftRender(origin);
        return NoteRender(origin + Point(1,0));
        break;
      case PIANO_PAD:
        OctaveShiftRender(origin);
        return PianoRender(origin + Point(1,0));
        break;
      case DRUM_PAD:
        return DrumRender(origin);
        break;
    }
    return false;
  }

    virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {
    
    bool ret = false;
    if (settingMode && xy.x > dimension.x - 6)
      ret = SettingAreaKeyEvent(xy, keyInfo);
    else if (*padType != DRUM_PAD && xy.x == 0)
      ret = OctaveShiftKeyEvent(xy, keyInfo);
    else
    {
      switch (*padType)
      {
        case NOTE_PAD:
          ret = NoteKeyEvent(xy, keyInfo);
          break;
        case PIANO_PAD:
          ret = PianoKeyEvent(xy, keyInfo);
          break;
        case DRUM_PAD:
          ret = DrumKeyEvent(xy, keyInfo);
          break;
      }
    }
    return ret;
    }

  void SettingAreaRender(Point origin)
  {
    Color noteColor  = COLOR_NOTE_PAD[1];
    Color pianoColor = COLOR_PIANO_PAD[1];
    Color drumColor  = (drumConfig != nullptr) ? COLOR_DRUM_PAD[1] : Color(BLANK);

    MatrixOS::LED::SetColor(origin + Point(dimension.x - 2, 0), noteColor.ToLowBrightness(*padType == NOTE_PAD));
    MatrixOS::LED::SetColor(origin + Point(dimension.x - 3, 0), pianoColor.ToLowBrightness(*padType == PIANO_PAD));
    MatrixOS::LED::SetColor(origin + Point(dimension.x - 4, 0), drumColor.ToLowBrightness(*padType == DRUM_PAD));
    MatrixOS::LED::SetColor(origin + Point(dimension.x - 5, 0), Color(BLANK));

    for (uint8_t y = 0; y < dimension.y; y++)
    {
      MatrixOS::LED::SetColor(origin + Point(dimension.x - 6,y), Color(BLANK));
    }
    for (uint8_t x = dimension.x - 5; x < dimension.x; x++)
    {
      for (uint8_t y = 1; y < dimension.y; y++)
      {
        Point xy = origin + Point(x, y);
        uint8_t i = (y - 1) * 4 + x - (dimension.x - 5);
        if (i < configCount)
        {
          Color color = (*padType == NOTE_PAD) ? noteColor : ((*padType == PIANO_PAD) ? pianoColor : drumColor);
          Color otherColor = (*padType == NOTE_PAD) ? pianoColor : noteColor;
          Color colorBack = (*padType != DRUM_PAD) ? Color(WHITE) : COLOR_DRUM_PAD[0];
          uint8_t* activePadConfig = &channelConfig->activePadConfig[channel][*padType];
          uint8_t* otherPadConfig = &channelConfig->activePadConfig[channel][(*padType == NOTE_PAD) ? PIANO_PAD : NOTE_PAD];
          if (i == *activePadConfig)
            MatrixOS::LED::SetColor(xy, color);
          else if (*padType != DRUM_PAD && i == *otherPadConfig)
            MatrixOS::LED::SetColor(xy, otherColor.ToLowBrightness());
          else
            MatrixOS::LED::SetColor(xy, colorBack.ToLowBrightness());
        }
        else
          MatrixOS::LED::SetColor(xy, Color(BLANK));
      }
    }
  }

  bool SettingAreaKeyEvent(Point xy, KeyInfo* keyInfo)
  {
    if(keyInfo->state == PRESSED){
      MatrixOS::FATFS::MarkChanged(channelConfig, 0);
      if ((xy.x == dimension.x - 1) && (xy.y == 0))
      {
        *padType = NOTE_PAD;
        GenerateNoteMap();
        return true;
      }
      else if ((xy.x == dimension.x - 2) && (xy.y == 0))
      {
        *padType = PIANO_PAD;
        GeneratePianoMap();
        return true;
      }
      else if ((drumConfig != nullptr) && (xy.x == dimension.x - 3) && (xy.y == 0))
      {
        *padType = DRUM_PAD;
        return true;
      }
      else if(xy.y > 0)
      {
        uint8_t i = (xy.y - 1) * 4 + xy.x - (dimension.x - 4);
        if (i < configCount && xy.x >= dimension.x - 4)
        {
          uint8_t* activePadConfig = &channelConfig->activePadConfig[channel][*padType];
          *activePadConfig = i;
          GenerateNoteMap();
          GeneratePianoMap();
          return true;
        }
        else
          return false;
      }
    }
    return false;
  }

};