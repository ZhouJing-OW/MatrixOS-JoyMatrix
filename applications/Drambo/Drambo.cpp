#include "Drambo.h"
#include "applications/BrightnessControl/BrightnessControl.h"


void Drambo::Setup() {
  if(!Inited || nvsVersion != DRAMBO_APP_VERSION) { // Set up configs
    bool configInit = ConfigInit();
    bool knobInit = KnobInit();
    if (configInit && knobInit) Inited = true;
  }

  Variable_Load(DRAMBO_ALL);

  transBar.Setup(&transState);
  knobBar.Setup(Dimension(8, 1), 8);
  tabBar.Setup(TAB, 5, activeTab, toggle);

  UI setup("", COLOR_BLUE);
  setup.SetLoopFunc([&]() -> void {
    if(MatrixOS::SYS::FNExit == true) Exit();
    toggleTab();
  });
  setup.AllowExit(false);
  setup.Start();
}

void Drambo::TabS(){
  UI tabS("Setting", COLOR_BLUE);
  Variable_Save(DRAMBO_ALL);

  UIButton systemSettingBtn("System Setting", COLOR_WHITE, [&]() -> void { MatrixOS::SYS::OpenSetting(); });
  // uint8_t map_length = sizeof(Device::fine_brightness_level) / sizeof(Device::fine_brightness_level[0]);
  // UITwoToneSelector brightnessSelector(
  //   Dimension(map_length, 1), map_length, COLOR_WHITE, COLOR_RED, 
  //   (uint8_t*)&MatrixOS::UserVar::brightness.value, 
  //   Device::brightness_level[4], 
  //   Device::fine_brightness_level,
  //   [&](uint8_t value) -> void { MatrixOS::UserVar::brightness.Set(value); }
  //   );
  UICross exitBtn("Exit", [&]() -> void { MatrixOS::SYS::FNExit = true;});
  // UIButtonLarge exitBtn("Exit", COLOR_RED, Dimension(2, 2), [&]() -> void { MatrixOS::SYS::FNExit = true;});
  UIButton formatBtn("Format", COLOR_RED, [&]() -> void { Device::FATFS::Format(); });
  UIButton KnobInitBtn("Knob Init", COLOR_ORANGE, [&]() -> void { KnobInit(); });

  tabS.AddUIComponent(systemSettingBtn,Point(15, 0));
  // setting.AddUIComponent(brightnessSelector,Point(2, 1));
  tabS.AddUIComponent(exitBtn,Point(0, 0));
  tabS.AddUIComponent(formatBtn,Point(15, 3));
  tabS.AddUIComponent(KnobInitBtn,Point(14, 3));

  CommonUI(tabS);
  tabS.SetLoopFunc([&]() -> void { if (toggle >= 0) tabS.Exit(); });
  tabS.SetEndFunc([&]() -> void {toggle = activeTab;});
  tabS.Start();
}

void Drambo::Tab0(){ // Main
  Knob_TogglePage(activeKnobPage);
  activePad = CH.type[MatrixOS::UserVar::global_MIDI_CH];
  Variable_Save(DRAMBO_CH); Variable_Save(DRAMBO_TAB);

  ChannelButton channel(Dimension(16, 1), &CH, &transState, true, 
    [&]() -> void { if(CH.type[MatrixOS::UserVar::global_MIDI_CH] != activePad)  toggle = 0;});
  MidiButton ccBtn(Dimension(8, 1), CC, 16);
  PianoPad piano(Dimension(15, 2), &PAD[0]);
  NotePad note(Dimension(15, 2), &PAD[1]);
  UIUpDown octave1(&PAD[0].octave , 9, 0, COLOR_WHITE, true, false);
  UIUpDown octave2(&PAD[1].octave , 9, 0, COLOR_WHITE, true, false);
  MidiButton drum1(Dimension(8, 2), DRUM, 16, &activeDrum, true , false);
  MidiButton drum2(Dimension(8, 2), DRUM, 16, &activeDrum, true , true);
  UIButtonWithColorFunc knobPage1("Page 1", 
    [&]()->Color{ Color color = COLOR_RED;    return activeKnobPage == 0 ? color : color.ToLowBrightness(); },
    [&]()->void{Knob_TogglePage(0);});
  UIButtonWithColorFunc knobPage2("Page 2", 
    [&]()->Color{ Color color = COLOR_PINK;   return activeKnobPage == 1 ? color : color.ToLowBrightness(); },
    [&]()->void{Knob_TogglePage(1);});
  UIButtonWithColorFunc knobPage3("Page 3", 
    [&]()->Color{ Color color = COLOR_VIOLET; return activeKnobPage == 2 ? color : color.ToLowBrightness(); },
    [&]()->void{Knob_TogglePage(2);});
  UIButtonWithColorFunc knobPage4("Page 4", 
    [&]()->Color{ Color color = COLOR_PURPLE; return activeKnobPage == 3 ? color : color.ToLowBrightness(); },
    [&]()->void{Knob_TogglePage(3);});

      UI tab0("");
  switch (activePad){
    case 0: // piano pad
      tab0.AddUIComponent(piano, Point(1, 2));
      tab0.AddUIComponent(octave1, Point(0, 2));
      break;
    case 1: // note pad
      tab0.AddUIComponent(note, Point(1, 2));
      tab0.AddUIComponent(octave2, Point(0, 2));
      break;
    case 2: // drum pad
      tab0.AddUIComponent(drum1, Point(0, 2));
      tab0.AddUIComponent(drum2, Point(8, 2));
      break;
  }
  tab0.AddUIComponent(channel, Point(0, 0));
  tab0.AddUIComponent(ccBtn, Point(4, 1));
  tab0.AddUIComponent(knobPage1, Point(12, 1));
  tab0.AddUIComponent(knobPage2, Point(13, 1));
  tab0.AddUIComponent(knobPage3, Point(14, 1));
  tab0.AddUIComponent(knobPage4, Point(15, 1));

  CommonUI(tab0);
  tab0.SetLoopFunc([&]() -> void { if (toggle >= 0) tab0.Exit(); });
  tab0.SetEndFunc([&]() -> void { Device::Encoder::DisableAll(); });
  tab0.Start();
}

void Drambo::Tab1(){ // Note
  Knob_TogglePage(activeKnobPage);

  PianoPad piano(Dimension(15, 4), &PAD[0]);
  NotePad note(Dimension(15, 4), &PAD[1]);
  UIUpDown octave1(&PAD[0].octave , 9, 0, COLOR_WHITE);
  UIUpDown octave2(&PAD[1].octave , 9, 0, COLOR_WHITE);

  UI tab1("");
  switch (TAB[1].subTab){
    case 0:
      tab1.AddUIComponent(note, Point(1, 0));
      tab1.AddUIComponent(octave2, Point(0, 2));
      break;
    case 1:
      tab1.AddUIComponent(piano, Point(1, 0));
      tab1.AddUIComponent(octave1, Point(0, 2));
      break;
  }

  CommonUI(tab1);
  tab1.SetLoopFunc([&]() -> void { if (toggle >= 0) tab1.Exit(); });
  tab1.SetEndFunc([&]() -> void { Device::Encoder::DisableAll(); });
  tab1.Start();
}

void Drambo::Tab2(){ // Sequncer
  UI tab2("");

  tab2.SetLoopFunc([&]() -> void { if (toggle >= 0) tab2.Exit(); });
  tab2.SetEndFunc([&]() -> void { Device::Encoder::DisableAll(); });

  CommonUI(tab2);
  tab2.Start();
}

void Drambo::Tab3(){ // Clips 
  UI tab3("");

  tab3.SetLoopFunc([&]() -> void { if (toggle >= 0) tab3.Exit(); });
  tab3.SetEndFunc([&]() -> void { Device::Encoder::DisableAll(); });

  CommonUI(tab3);
  tab3.Start();
}

void Drambo::Tab4(){ // Mixer
  UI tab4("");

  tab4.SetLoopFunc([&]() -> void { if (toggle >= 0) tab4.Exit(); });
  tab4.SetEndFunc([&]() -> void { Device::Encoder::DisableAll(); });

  CommonUI(tab4);
  tab4.Start();
}

void Drambo::Pop(){
  UI pop("");

  pop.AddUIComponent(knobBar, Point(8, 4));
  pop.Start();
}

void Drambo::toggleTab(){
  switch (toggle) {
    case -1: TabS(); break;
    case 0: toggle = -1; activeTab = 0; Tab0(); break;
    case 1: toggle = -1; activeTab = 1; Tab1(); break;
    case 2: toggle = -1; activeTab = 2; Tab2(); break;
    case 3: toggle = -1; activeTab = 3; Tab3(); break;
    case 4: toggle = -1; activeTab = 4; Tab4(); break;
    case 9: toggle = -1; activeTab = 9; Pop() ; break;
  }
}

void Drambo::CommonUI(UI &ui){
  ui.AddUIComponent(transBar, Point(0, 4));
  ui.AddUIComponent(tabBar, Point(3, 4));
  ui.AddUIComponent(knobBar, Point(8, 4));
}

bool Drambo::Variable_Load(uint8_t dataID){
  if (dataID < DRAMBO_ALL){
    int8_t load = MatrixOS::NVS::GetVariable(configHash[dataID], configPt[dataID], configSize[dataID]);
    return (load == 0);
  }
  else if (dataID == DRAMBO_ALL)
  {
    int8_t load = 0;
    for (int i = 0; i < DRAMBO_ALL; i++){
      load += MatrixOS::NVS::GetVariable(configHash[i], configPt[i], configSize[i]);
    }
    return (load == 0);
  }
  return false;
}

bool Drambo::Variable_Save(uint8_t dataID){
  if (dataID < DRAMBO_ALL){
    return MatrixOS::NVS::SetVariable(configHash[dataID], configPt[dataID], configSize[dataID]);
  } else if(dataID == DRAMBO_ALL){
    uint8_t saved = 0;
    for (int i = 0; i < DRAMBO_ALL; i++){
      saved += MatrixOS::NVS::SetVariable(configHash[i], configPt[i], configSize[i]);
    }
    return (saved == DRAMBO_ALL);
  }
  return false;
}

void Drambo::Knob_Switch(uint16_t ID[ENCODER_NUM]){ //if ID == 0xFFFF , it will be blocked
  std::fstream fio;
  fio.open(DRAMBO_KNOB_PATH, std::ios::in | std::ios:: out | std::ios::binary);
  if (!fio.is_open()) {
    MLOGD("FatFs", "Failed to open Knobs configuration.");
    return;
  } // MLOGD("FatFs", "Succeeded to open Knobs configuration.");

  for (uint8_t i = 0; i < ENCODER_NUM; i++) {
    if((knobBar.knobID[i] != 0xFFFF) && (knobBar.knob[i].changed == true)) {
      knobBar.knob[i].changed = false;
      uint8_t ch = knobBar.knobID[i] >> 8;
      uint8_t n = knobBar.knobID[i] & 0xFF;
      fio.seekp((ch * DRAMBO_NUM_OF_KNOB + n) * sizeof(KnobConfig), std::ios::beg);
      fio.write((char*)&knobBar.knob[i], sizeof(KnobConfig)); 
    }
  } 

  for (uint8_t i = 0; i < ENCODER_NUM; i++) {
    uint8_t ch = ID[i] >> 8;
    uint8_t n = ID[i] & 0xFF;
    fio.seekg((ch * DRAMBO_NUM_OF_KNOB + n) * sizeof(KnobConfig), std::ios::beg);
    fio.read((char*)&knobBar.knob[i], sizeof(KnobConfig));
    Device::Encoder::Setup(&knobBar.knob[i], i);
    if (ID[i] == 0xFFFF) Device::Encoder::Disable(i);
    knobBar.knobID[i] = ID[i];
  }
  fio.close();
}

void Drambo::Knob_TogglePage(uint8_t page){
  uint16_t loadKnob[ENCODER_NUM];
  for (uint8_t i = 0; i < ENCODER_NUM; i++) {
    uint8_t ch = MatrixOS::UserVar::global_MIDI_CH;
    uint8_t n = page * ENCODER_NUM + i;
    loadKnob[i] = ch << 8 | n;
  }
  Knob_Switch(loadKnob);
  activeKnobPage = page;
}

bool Drambo::ConfigInit(){

  for(uint8_t ch = 0; ch < 16; ch++){
    CH.color[ch] = COLOR_LIME;
    CH.type[ch] = 0;
  }
  CH.type[1]  = 2;
  CH.type[2]  = 2;
  CH.color[0]  = COLOR_PURPLE;
  CH.color[1]  = COLOR_ORANGE;
  CH.color[2]  = COLOR_ORANGE;
  CH.color[13] = COLOR_AZURE;
  CH.color[14] = COLOR_AZURE;
  CH.color[15] = COLOR_PURPLE;

  TAB[0] = TabConfig{"MAIN", COLOR_LIME, 0, 3};
  TAB[1] = TabConfig{"NOTE", COLOR_PURPLE, 0, 2};
  TAB[2] = TabConfig{"SEQ", COLOR_CYAN, 0, 1};
  TAB[3] = TabConfig{"CLIP", COLOR_VIOLET, 0, 1};
  TAB[4] = TabConfig{"MIXER", COLOR_YELLOW,0, 2};

  PAD[0] = {.color = COLOR_AZURE, .rootColor = COLOR_BLUE, .type = PIANO_PAD, .scale = CHROMATIC};
  PAD[1] = {.color = COLOR_PURPLE, .rootColor = COLOR_PINK , .type = NOTE_PAD, .overlap = 2, .scale = NATURAL_MINOR};

  for (uint8_t n = 0; n < 16; n++) {
    DRUM[n].byte1 = n + 36;
    DRUM[n].channel = 9;
    DRUM[n].color = COLOR_ORANGE;
    DRUM[n].type = SEND_NOTE;
    DRUM[n].globalChannel = true;
  }
  DRUM[0].color = COLOR_RED;
  DRUM[12].color = COLOR_RED;

  for (uint8_t n = 0; n < 16; n++) {
    CC[n].byte1 = 2;
    CC[n].channel = n;
    if (n < 8 ) CC[n].color = COLOR_YELLOW; else CC[n].color = COLOR_ORANGE;
    CC[n].globalChannel = false;
  }

  Variable_Save(DRAMBO_ALL);
  return true;
}

bool Drambo::KnobInit(){
  
  std::ofstream fout;
  fout.open(DRAMBO_KNOB_PATH, std::ios::binary);
  if (!fout.is_open()){
    MLOGD("FatFs", "Failed to init Knobs configuration.");
    return false;
  } // MLOGD("FatFs", "Succeeded to init  Knobs configuration.");
  
  for (uint8_t ch = 0; ch < 16; ch++) {
    for (uint8_t n = 0; n < DRAMBO_NUM_OF_KNOB; n++) {
      KnobConfig tempKnob;
      tempKnob.channel = ch;
      tempKnob.byte1 = n + 16;
      tempKnob.type = SEND_CC;
      tempKnob.enable = true;
      switch (n) {
        case 4: tempKnob.lock = true,  tempKnob.byte1 = 8; break;
        case 5: tempKnob.lock = true, tempKnob.byte1 = 9; break;
        case 6: tempKnob.lock = true, tempKnob.byte1 = 10; tempKnob.byte2 = 63; tempKnob.def = 63; break;
        case 7: tempKnob.lock = true, tempKnob.byte1 = 7, tempKnob.byte2 = 78, tempKnob.def = 78; break;
      }
      switch (n / ENCODER_NUM) {
        case 0: tempKnob.color = COLOR_RED; break;
        case 1: tempKnob.color = COLOR_PINK; break;
        case 2: tempKnob.color = COLOR_VIOLET; break;
        case 3: tempKnob.color = COLOR_PURPLE; break;
      }
      fout.write((char*)&tempKnob, sizeof(KnobConfig));
    }
  }
  
  fout.close();
  return true;
}
