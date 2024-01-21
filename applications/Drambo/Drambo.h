#pragma once

#include "MatrixOS.h"
#include "UI/UI.h"
#include "applications/Application.h"
#include "framework/Scales.h"
#include "framework/ComponentVariable.h"
#include <fstream>



#define DRAMBO_APP_VERSION  1
#define DRAMBO_CONFIG_HASH  StaticHash("ZhouJing-Drambo-Config")
#define DRAMBO_CONFIG_PATH  "/storage/Drambo-c.cfg"
#define DRAMBO_KNOB_PATH    "/storage/Drambo-k.cfg"
#define DRAMBO_NUM_OF_KNOB  32

class Drambo : public Application {
  public:
  static Application_Info info;

  // Saved Variables
  CreateSavedVar("Drambo", nvsVersion, uint32_t, DRAMBO_APP_VERSION);
  CreateSavedVar("DramboInit", Inited, bool, false);

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
  bool Variable_Load(uint8_t dataID);
  bool Variable_Save(uint8_t dataID);
  void Knob_SwitchActive(uint16_t ID[8]);
  void Knob_TogglePage(uint8_t page);
  bool ConfigInit();
  bool KnobInit();


  ChannelConfig CH;
  TabConfig TAB[5];
  NotePadConfig PAD[2];
  MidiButtonConfig DRUM[16];
  MidiButtonConfig CC[8];
  MidiButtonConfig PT[16];

  enum configID : uint8_t {  
    DRAMBO_CH = 0, 
    DRAMBO_TAB = 1, 
    DRAMBO_PAD = 2, 
    DRAMBO_DRUM = 3, 
    DRAMBO_CC = 4, 
    DRAMBO_PT = 5, 
    DRAMBO_ALL = 6 
  };

  void* configPt[6] = {
    (void*)&CH, 
    (void*)&TAB, 
    (void*)&PAD, 
    (void*)&DRUM, 
    (void*)&CC, 
    (void*)&PT
  };

  uint32_t configHash[6] = {
    StaticHash("ZhouJing-Drambo-CH"),
    StaticHash("ZhouJing-Drambo-TAB"), 
    StaticHash("ZhouJing-Drambo-PAD"), 
    StaticHash("ZhouJing-Drambo-DRUM"), 
    StaticHash("ZhouJing-Drambo-CC"), 
    StaticHash("ZhouJing-Drambo-PT")
  };

  size_t configSize[6] = {
    sizeof(CH), 
    sizeof(TAB), 
    sizeof(PAD), 
    sizeof(DRUM), 
    sizeof(CC), 
    sizeof(PT)
  };

  uint16_t activeKnobID[8] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
  uint8_t knobValAtLoad[8];
  KnobConfig activeKnob[8];
  int16_t activeKnobVal[8];
  MainBar mainBar;
  TabBar tabBar;
  KnobButton knobBar;
  
  int8_t activeTab = 0;
  int8_t activeDrum;
  int8_t activeCC;
  int8_t activePT;
  int8_t toggle = 0;
  uint8_t activeKnobPage = 0;
};

inline Application_Info Drambo::info = {
  .name = "Drambo",
  .author = "ZhouJing",
  .color = COLOR_BLUE,
  .version = DRAMBO_APP_VERSION,
  .visibility = true,
};

REGISTER_APPLICATION(Drambo);