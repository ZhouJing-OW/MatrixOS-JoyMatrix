#pragma once

#include "MatrixOS.h"
#include "UI/UI.h"
#include "applications/Application.h"
#include "framework/Scales.h"
#include "framework/ComponentVariable.h"
#include "system/MIDIAPP/MidiCenter.h"
#include "system/MIDIAPP/MidiAppUI.h"
#include <fstream>

// #define DRAMBO_CONFIG_HASH  StaticHash("ZhouJing-Drambo-Config")
#define CONFIG_SUFFIX "cfg"

constexpr uint32_t DRAMBO_APP_VERSION = 1;
constexpr uint8_t  DRAMBO_NUM_OF_KNOB = 32;

class Drambo : public Application {
  public:
  static Application_Info info;

  // Saved Variables
  CreateSavedVar("Drambo", nvsVersion, uint32_t, DRAMBO_APP_VERSION);
  CreateSavedVar("DramboInit", Inited, bool, false);

  ChannelConfig*    CH;
  TabConfig*        TAB;
  // NotePadConfig*    PAD;
  // MidiButtonConfig* DRUM;
  MidiButtonConfig* CC;
  MidiButtonConfig* PT;

  std::list<SaveVarInfo> saveVarList = 
  {
    // SaveVarInfo {(void**)&CH,   sizeof(ChannelConfig),    1},
    SaveVarInfo {(void**)&TAB,  sizeof(TabConfig),        5},
    // SaveVarInfo {(void**)&PAD,  sizeof(NotePadConfig),    4},
    // SaveVarInfo {(void**)&DRUM, sizeof(MidiButtonConfig), 4 * 16},
    SaveVarInfo {(void**)&CC,   sizeof(MidiButtonConfig), 16},
    SaveVarInfo {(void**)&PT,   sizeof(MidiButtonConfig), 16},
  };

  //common bar
  TransBar transBar;
  TabBar tabBar;

  MixerFader volumeMixer;
  KnobButton mixer;
  std::vector<uint16_t> volumeMixerPos;
  std::vector<uint16_t> mixerPos;

  //current state
  int8_t activeTab = 0;
  int8_t activePT;
  int8_t toggle = 0;

  void Setup() override;
  void TabS();
  void Tab0();
  void Tab1();
  void Tab2();
  void Tab3();
  void Tab4();
  void Pop();

  void toggleTab();
  void CommonUI(UI& ui);
  void CommonLoop(UI& ui);
  void CommonEnd(UI& ui);
  bool ConfigInit();
  bool KnobInit();
  void ExitApp();
};

inline Application_Info Drambo::info = {
  .name = "Drambo",
  .author = "ZhouJing",
  .color = COLOR_BLUE,
  .version = DRAMBO_APP_VERSION,
  .visibility = true,
};

REGISTER_APPLICATION(Drambo);