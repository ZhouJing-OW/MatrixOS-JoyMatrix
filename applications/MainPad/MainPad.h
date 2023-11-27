#pragma once

#include "MatrixOS.h"
#include "applications/Note/Scales.h"
#include "UI/UI.h"
#include "applications/Application.h"



#define MAINPAD_APP_VERSION 1
#define MAINPAD_CONFIGS_HASH StaticHash("ZhouJing-Main-MainPadConfigs")

struct MainPadConfig {

  public:
    uint16_t scale = CHROMATIC;
    uint8_t rootKey = 0;
    int8_t octave = 3;
    uint8_t channel = 0;
    uint8_t overlap = 4;
    Color color = Color(0x00FFFF);
    Color rootColor = Color(0x0040FF);

    bool enfourceScale = true;
    bool alignRoot = true;  // Only works when overlap is set to 0
    bool followChannelChange = false;

    bool emptyBit[5]; //空位，用于对齐16bit
};

#define CC_CONFIGS_HASH StaticHash("ZhouJing-Main-buttonConfigs")

struct ButtonConfig {

  public:
  uint8_t type[4][8] = {0};  // 0: Command Change, 1: Program Change , 2: Note
  uint8_t channel[4][8] = {0}; // channel
  uint8_t value1[4][8] = {0}; // CC
  uint8_t value2[4][8] = {127}; // value
  uint8_t activeButton[4] = {0}; // 4 button groups
  uint8_t activeGroup = 0;
  Color color[4] = {Color(0x0099FF)};

  // channel button
  Color channelColor[16] = {Color(0x00FF00)};
  uint8_t channelCH[16] = {0};
  uint8_t channelSelect[16] = {120};  // CC
  uint8_t channelMute[16] = {119}; // CC
  uint8_t channelSolo[16] = {118}; // CC
  uint8_t activeChannel = 0;
  uint8_t channelGroupDivide = 1;
};

#define KNOB_CONFIGS_HASH StaticHash("ZhouJing-Main-KnobConfigs")

struct KnobConfig {

  public:   
   uint8_t activeGroup = 0;
   uint8_t channel[4][8] = {0}; //channel
   uint8_t value1[4][8]; // CC
   int32_t value2[4][8][16]; // value
   int32_t defaultValue[4][8];
   Color color[4] = {Color(0x9900FF)};
   bool followChannelChange[4] = {true};

   bool emptyBit[4]; //空位，用于对齐8bit
};

class MainPad : public Application {
 public:
  static Application_Info info;

  // Saved Variables
  CreateSavedVar("MainPad", nvsVersion, uint32_t, MAINPAD_APP_VERSION); 
  CreateSavedVar("MainPad", activeConfig, uint8_t, 0);
  CreateSavedVar("MainPad", lastActiveConfig1, uint8_t, 0);
  CreateSavedVar("MainPad", lastActiveConfig2, uint8_t, 1);
  CreateSavedVar("MainPad", splitView, bool, false);

  CreateSavedVar("MainPad", emptybit2, bool, 0); //空位，用于对齐8bit
  CreateSavedVar("MainPad", emptybit3, bool, 0);
  CreateSavedVar("MainPad", emptybit4, bool, 0);
  CreateSavedVar("MainPad", emptybit5, bool, 0);
  CreateSavedVar("MainPad", emptybit6, bool, 0);
  CreateSavedVar("MainPad", emptybit7, bool, 0);
  CreateSavedVar("MainPad", emptybit8, bool, 0);

  void Setup() override;

  void KeyEventHandler(uint16_t keyID, KeyInfo* keyInfo);

  void GridKeyEvent(Point xy, KeyInfo* KeyInfo);
  void IDKeyEvent(uint16_t keyID, KeyInfo* KeyInfo);

  void MainView();
  void Note();
  void Mixer1();
  void Mixer2();

  void ButtonSettings();
  void ChannelBtnSettings();
  void KnobSettings();
  void VelocitySettings();
  void ScaleSelector();
  void OverlapSelector();
  void ChannelSelector(Color color, uint8_t* channel);
  void ColorSelector(uint8_t con);
  void NoteSelector(uint8_t group, uint8_t number);
  void KnobSetup(uint8_t type);
  void MixerKnobSetup(bool active, uint8_t i);

  void FuncPage(uint8_t page);
  void LoadVariable();
  void SaveVariable();

  MainPadConfig mainPadConfigs[2];
  ButtonConfig buttonConfigs[1];
  KnobConfig knobConfigs[1];

  uint8_t funcPage = 0;
  uint8_t lastPage = 1;

  uint16_t scales[32] = {NATURAL_MINOR,
                         MAJOR,
                         DORIAN,
                         PHRYGIAN,
                         MIXOLYDIAN,
                         MELODIC_MINOR_ASCENDING,
                         HARMONIC_MINOR,
                         BEBOP_DORIAN,
                         BLUES,
                         MINOR_PENTATONIC,
                         HUNGARIAN_MINOR,
                         UKRANIAN_DORIAN,
                         MARVA,
                         TODI,
                         WHOLE_TONE,
                         CHROMATIC,
                         LYDIAN,
                         LOCRIAN,
                         MAJOR_PENTATONIC,
                         PHYRIGIAN_DOMINATE,
                         HALF_WHOLE_DIMINISHED,
                         MIXOLYDIAN_BEBOP,
                         SUPER_LOCRIAN,
                         HIRAJOSHI,
                         IN_SEN,
                         YO_SCALE,
                         IWATO,
                         WHOLE_HALF,
                         BEBOP_MINOR,
                         MAJOR_BLUES,
                         KUMOI,
                         BEBOP_MAJOR};

  string scale_names[32] = {"Natural Minor",
                            "Major",
                            "Dorian",
                            "Phrygian",
                            "Mixolydian",
                            "Melodic Minor Ascending",
                            "Harmonic Minor",
                            "Bebop Dorian",
                            "Blues",
                            "Minor Pentatonic",
                            "Hungarian Minor",
                            "Ukranian Dorian",
                            "Marva",
                            "Todi",
                            "Whole Tone",
                            "Chromatic",
                            "Lydian",
                            "Locrian",
                            "Major Pentatonic",
                            "Phyrigian Dominate",
                            "Half-Whole Diminished",
                            "Mixolydian BeBop",
                            "Super Locrian",
                            "Hirajoshi",
                            "In Sen",
                            "Yo Scale",
                            "Iwato",
                            "Whole Half",
                            "BeBop Minor",
                            "Major Blues",
                            "Kumoi",
                            "BeBop Major"};
};

inline Application_Info MainPad::info = {
    .name = "Main Pad",
    .author = "ZhouJing",
    .color =  Color(0x0000FF),
    .version = MAINPAD_APP_VERSION,
    .visibility = true,
};

REGISTER_APPLICATION(MainPad);