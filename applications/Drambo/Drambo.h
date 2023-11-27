#pragma once

#include "MatrixOS.h"
#include "UI/UI.h"
#include "applications/Application.h"
#include "framework/Scales.h"
#include "framework/ComponentVariable.h"



#define DRAMBO_APP_VERSION 1
#define DRAMBO_CONFIGS_HASH StaticHash("ZhouJing-Drambo-Configs")

class Drambo : public Application {
 public:
  static Application_Info info;

  // Saved Variables
  CreateSavedVar("MainPad", nvsVersion, uint32_t, DRAMBO_APP_VERSION); 
  CreateSavedVar("MainPad", activePad, uint8_t, 0);
  CreateSavedVar("MainPad", activeCC, uint8_t, 0);
  CreateSavedVar("MainPad", activePT, uint8_t, 0);
  CreateSavedVar("MainPad", activeKnob, uint8_t, 0);

  void Setup() override;
  void Tab0();
  void Tab1();
  void Tab2();
  void Tab3();
  void Tab4();
  void Pop();

  void LoadVariable();
  void SaveVariable();

  struct Config {
    TabConfig Tab[5];
    NotePadConfig Pad[2];
    ChannelButtonConfig CH[16];
    MidiButtonConfig Note[16];
    MidiButtonConfig CC[8];
    MidiButtonConfig PT[16];
    KnobConfig Knob[16][32];
  };

  Config config;
  int8_t activeTab = 0;
  int8_t toggle = 0;
  
  MainBar mainBar;
  TabBar tabBar;
  KnobButton knobBar;
};

inline Application_Info Drambo::info = {
  .name = "Drambo",
  .author = "ZhouJing",
  .color = COLOR_BLUE,
  .version = DRAMBO_APP_VERSION,
  .visibility = true,
};

REGISTER_APPLICATION(Drambo);