#pragma once

#include "MatrixOS.h"
#include "UI/UI.h"
#include "applications/Application.h"
#include "framework/Scales.h"
#include "framework/ComponentVariable.h"
#include <fstream>


// #define DRAMBO_CONFIG_HASH  StaticHash("ZhouJing-Drambo-Config")
// #define DRAMBO_CONFIG_PATH  "/storage/Drambo-c.cfg"
#define DRAMBO_KNOB_PATH    "/storage/Drambo-k.cfg"
constexpr uint32_t DRAMBO_APP_VERSION = 1;
constexpr uint8_t  DRAMBO_NUM_OF_KNOB = 32;


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
  void Knob_SaveLoad(std::vector<KnobConfig>& knob, std::vector<uint16_t>& knobID, std::vector<uint16_t>& newID);
  void KnobBar_Toggle(uint8_t page);
  void SetEncoderPtr(std::vector<KnobConfig>& knob, uint8_t offset = 0);
  bool ConfigInit();
  bool KnobInit();


  ChannelConfig CH;
  TabConfig TAB[5];
  NotePadConfig PAD[2];
  MidiButtonConfig DRUM[16];
  MidiButtonConfig CC[16];
  MidiButtonConfig PT[16];
  PCBankConfig PC;

  enum configID : uint8_t {  
    DRAMBO_CH = 0,    DRAMBO_TAB = 1,   DRAMBO_PAD = 2,   DRAMBO_DRUM = 3,    DRAMBO_CC = 4,
    DRAMBO_PT = 5,    DRAMBO_PC = 6,DRAMBO_ALL = 7, };

  void* configPt[DRAMBO_ALL] = {
    (void*)&CH,       (void*)&TAB,      (void*)&PAD,      (void*)&DRUM,       (void*)&CC,         
    (void*)&PT,       (void*)&PC };

  size_t configSize[DRAMBO_ALL] = {
    sizeof(CH),       sizeof(TAB),      sizeof(PAD),      sizeof(DRUM),       sizeof(CC), 
    sizeof(PT),       sizeof(PC) };
  
  uint32_t configHash[DRAMBO_ALL] = {
    StaticHash("ZhouJing-Drambo-CH"),
    StaticHash("ZhouJing-Drambo-TAB"), 
    StaticHash("ZhouJing-Drambo-PAD"), 
    StaticHash("ZhouJing-Drambo-DRUM"), 
    StaticHash("ZhouJing-Drambo-CC"), 
    StaticHash("ZhouJing-Drambo-PT"),
    StaticHash("ZhouJing-Drambo-PC"),
  };

  //common bar
  TransBar transBar;
  TabBar tabBar;
  KnobButton knobBar;
  TransportState transState;

  MixerFader volumeMix;
  KnobButton panMix;
  KnobButton sendAMix;
  KnobButton sendBMix;
  vector<uint16_t> volumeID;
  vector<uint16_t> panID;
  vector<uint16_t> sendAID;
  vector<uint16_t> sendBID;
  vector<uint16_t> exitID; // keep empty

  //current state
  int8_t activeTab = 0;
  int8_t activePad = 0;
  int8_t activeDrum;
  int8_t activeCC;
  int8_t activePT;
  int8_t toggle = 0;
  int8_t activeKnobPage = 0;
  
};

inline Application_Info Drambo::info = {
  .name = "Drambo",
  .author = "ZhouJing",
  .color = COLOR_BLUE,
  .version = DRAMBO_APP_VERSION,
  .visibility = true,
};

REGISTER_APPLICATION(Drambo);