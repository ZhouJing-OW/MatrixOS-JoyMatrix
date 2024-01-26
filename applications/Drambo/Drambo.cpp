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
  knobBar.Setup(Dimension(8, 1), 8, false, true);
  tabBar.Setup(TAB, 5, activeTab, toggle);

  volumeMix.Setup(Dimension(16, 4), 16, true, true, [&]()->void { SetEncoderPtr(volumeMix.knob, volumeMix.activePoint);
    panMix.activePoint = -ENCODER_NUM; sendAMix.activePoint = -ENCODER_NUM; sendBMix.activePoint = -ENCODER_NUM;
  }); 
  panMix.Setup(Dimension(16, 1), 16, true, false, [&]()->void { SetEncoderPtr(panMix.knob, panMix.activePoint); 
    volumeMix.activePoint = -ENCODER_NUM; sendAMix.activePoint = -ENCODER_NUM; sendBMix.activePoint = -ENCODER_NUM;
  }); 
  sendAMix.Setup(Dimension(16,1), 16, true, false, [&]()->void { SetEncoderPtr(sendAMix.knob, sendAMix.activePoint); 
    volumeMix.activePoint = -ENCODER_NUM; panMix.activePoint = -ENCODER_NUM; sendBMix.activePoint = -ENCODER_NUM;
  }); 
  sendBMix.Setup(Dimension(16, 1), 16, true, false, [&]()->void { SetEncoderPtr(sendBMix.knob, sendBMix.activePoint); 
    volumeMix.activePoint = -ENCODER_NUM; panMix.activePoint = -ENCODER_NUM; sendAMix.activePoint = -ENCODER_NUM;
  }); 

  for (uint8_t ch = 0; ch < 16; ch++){
    uint16_t vID = ch << 8 | 7;   uint16_t pID = ch << 8 | 6;   uint16_t aID = ch << 8 | 4;   uint16_t bID = ch << 8 | 5;
    volumeID.push_back(vID);      panID.push_back(pID);         sendAID.push_back(aID);       sendBID.push_back(bID);
  }

  Knob_SaveLoad(volumeMix.knob, volumeMix.knobID, volumeID);
  Knob_SaveLoad(panMix.knob, panMix.knobID, panID);
  Knob_SaveLoad(sendAMix.knob, sendAMix.knobID, sendAID);
  Knob_SaveLoad(sendBMix.knob, sendBMix.knobID, sendBID);

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
  tabS.Start();

  // exit Function
  toggle = activeTab;
}

void Drambo::Tab0(){ // Main
  KnobBar_Toggle(activeKnobPage);
  Device::AnalogInput::SetLeftRight(&activeKnobPage, 3, 0, true, [&]() -> void { KnobBar_Toggle(activeKnobPage); });
  uint8_t chn = MatrixOS::UserVar::global_MIDI_CH;
  activePad = CH.type[chn];
  Variable_Save(DRAMBO_CH); Variable_Save(DRAMBO_TAB);
  UIPlusMinus ProgramBank(&PC.bank[chn], 127, 1, COLOR_CYAN, false,
                       [&]() -> void { MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, chn, 0, PC.bank[chn])); });
  UIPlusMinus ProgramNum(&PC.pc[chn], 127, 0, COLOR_AZURE, false,
    [&]()-> void { MatrixOS::MIDI::Send(MidiPacket(0, ProgramChange, chn, PC.pc[chn], 0));});

  ChannelButton channel(Dimension(16, 1), &CH, &transState, true, [&]() -> void { 
    chn = MatrixOS::UserVar::global_MIDI_CH;
    ProgramBank.val = &PC.bank[chn];
    ProgramNum.val = &PC.pc[chn];
    KnobBar_Toggle(activeKnobPage);
    if (CH.type[chn] != activePad) toggle = 0; });
  
  MidiButton ccBtn(Dimension(8, 1), CC, 16);
  PianoPad piano(Dimension(15, 2), &PAD[0]);
  NotePad note(Dimension(15, 2), &PAD[1]);
  UIPlusMinus octave1(&PAD[0].octave , 9, 0, COLOR_WHITE, true);
  UIPlusMinus octave2(&PAD[1].octave , 9, 0, COLOR_WHITE, true);
  MidiButton drum1(Dimension(8, 2), DRUM, 16, &activeDrum, true , false);
  MidiButton drum2(Dimension(8, 2), DRUM, 16, &activeDrum, true , true);
  UIButtonWithColorFunc knobPage1("Page 1", 
    [&]()->Color{ Color color = COLOR_RED;    return activeKnobPage == 0 ? color : color.ToLowBrightness(); },
    [&]()->void{KnobBar_Toggle(0);});
  UIButtonWithColorFunc knobPage2("Page 2", 
    [&]()->Color{ Color color = COLOR_PINK;   return activeKnobPage == 1 ? color : color.ToLowBrightness(); },
    [&]()->void{KnobBar_Toggle(1);});
  UIButtonWithColorFunc knobPage3("Page 3", 
    [&]()->Color{ Color color = COLOR_VIOLET; return activeKnobPage == 2 ? color : color.ToLowBrightness(); },
    [&]()->void{KnobBar_Toggle(2);});
  UIButtonWithColorFunc knobPage4("Page 4", 
    [&]()->Color{ Color color = COLOR_PURPLE; return activeKnobPage == 3 ? color : color.ToLowBrightness(); },
    [&]()->void{KnobBar_Toggle(3);});

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
  tab0.AddUIComponent(ProgramBank, Point(0, 1));
  tab0.AddUIComponent(ProgramNum, Point(2, 1));
  tab0.AddUIComponent(ccBtn, Point(4, 1));
  tab0.AddUIComponent(knobPage1, Point(12, 1));
  tab0.AddUIComponent(knobPage2, Point(13, 1));
  tab0.AddUIComponent(knobPage3, Point(14, 1));
  tab0.AddUIComponent(knobPage4, Point(15, 1));

  CommonUI(tab0);
  tab0.SetLoopFunc([&]() -> void { if (toggle >= 0) tab0.Exit(); });
  tab0.Start();
}

void Drambo::Tab1(){ // Note
  KnobBar_Toggle(activeKnobPage);
  Device::AnalogInput::SetLeftRight(&activeKnobPage, 3, 0, true, [&]() -> void { KnobBar_Toggle(activeKnobPage); });

  PianoPad piano(Dimension(15, 4), &PAD[0]);
  NotePad note(Dimension(15, 4), &PAD[1]);
  UIPlusMinus octave1(&PAD[0].octave , 9, 0, COLOR_WHITE);
  UIPlusMinus octave2(&PAD[1].octave , 9, 0, COLOR_WHITE);

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
  tab1.Start();
}

void Drambo::Tab2(){ // Sequncer
  UI tab2("");

  CommonUI(tab2);
  tab2.SetLoopFunc([&]() -> void { if (toggle >= 0) tab2.Exit(); });
  tab2.Start();
}

void Drambo::Tab3(){ // Clips 
  UI tab3("");

  CommonUI(tab3);
  tab3.SetLoopFunc([&]() -> void { if (toggle >= 0) tab3.Exit(); });
  tab3.Start();
}

void Drambo::Tab4(){ // Mixer
  UI tab4("");

  if (TAB[4].subTab == 0) volumeMix.dimension = Dimension(16, 4);
  else volumeMix.dimension = Dimension(16, 1);
  SetEncoderPtr(volumeMix.knob);

  volumeMix.activePoint = 0;
  panMix.activePoint = -ENCODER_NUM;
  sendAMix.activePoint = -ENCODER_NUM;
  sendBMix.activePoint = -ENCODER_NUM;

  for(uint8_t ch = 0; ch < 16; ch++){
    volumeMix.knob[ch].color = CH.color[ch];
    panMix.knob[ch].color = COLOR_RED;
    sendAMix.knob[ch].color = COLOR_GREEN;
    sendBMix.knob[ch].color = COLOR_GREEN;
  }

  tab4.AddUIComponent(volumeMix, Point(0, 0));
  if(TAB[4].subTab == 1) {
    tab4.AddUIComponent(panMix, Point(0, 1));
    tab4.AddUIComponent(sendAMix, Point(0, 2));
    tab4.AddUIComponent(sendBMix, Point(0, 3));
  }

  CommonUI(tab4);
  tab4.SetLoopFunc( [&]() -> void { if (toggle >= 0) tab4.Exit(); });
  tab4.Start();

  // exit Function
  for(uint8_t ch = 0; ch < 16; ch++){
    volumeMix.knob[ch].color = COLOR_RED;
    panMix.knob[ch].color = COLOR_RED;
    sendAMix.knob[ch].color = COLOR_RED;
    sendBMix.knob[ch].color = COLOR_RED;
  }

  Knob_SaveLoad(volumeMix.knob, volumeMix.knobID, exitID);
  Knob_SaveLoad(panMix.knob, panMix.knobID, exitID);
  Knob_SaveLoad(sendAMix.knob, sendAMix.knobID, exitID);
  Knob_SaveLoad(sendBMix.knob, sendBMix.knobID, exitID);
}

void Drambo::Pop(){
  UI pop("");

  pop.AddUIComponent(knobBar, Point(8, 4));
  pop.Start();
}

void Drambo::toggleTab()
{
  Knob_SaveLoad(knobBar.knob, knobBar.knobID, exitID);
  Device::Encoder::DisableAll();
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

void Drambo::CommonUI(UI &ui)
{
  ui.AddUIComponent(transBar, Point(0, 4));
  ui.AddUIComponent(tabBar, Point(3, 4));
  ui.AddUIComponent(knobBar, Point(8, 4));
}

bool Drambo::Variable_Load(uint8_t dataID)
{
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

bool Drambo::Variable_Save(uint8_t dataID)
{
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

void Drambo::Knob_SaveLoad(std::vector<KnobConfig> &knob, std::vector<uint16_t> &knobID, std::vector<uint16_t> &newID) 
{ 
  std::fstream fio;
  fio.open(DRAMBO_KNOB_PATH, std::ios::in | std::ios:: out | std::ios::binary);
  if (!fio.is_open()) {
    MLOGD("FatFs", "Failed to open Knobs configuration.");
    return;
  } // MLOGD("FatFs", "Succeeded to open Knobs configuration.");

  for (uint8_t i = 0; i < knob.size(); i++) {
    if((knobID[i] != 0xFFFF) && (knob[i].changed == true)) { //if ID == 0xFFFF , it will not be saved.
      knob[i].changed = false;
      uint8_t ch = knobID[i] >> 8;
      uint8_t n = knobID[i] & 0xFF;
      fio.seekp((ch * DRAMBO_NUM_OF_KNOB + n) * sizeof(KnobConfig), std::ios::beg);
      fio.write((char*)&knob[i], sizeof(KnobConfig)); 
    }
  } 

  for (uint8_t i = 0; i < newID.size(); i++) { // if the newID is empty, do not load.
    uint8_t ch = newID[i] >> 8;
    uint8_t n = newID[i] & 0xFF;
    fio.seekg((ch * DRAMBO_NUM_OF_KNOB + n) * sizeof(KnobConfig), std::ios::beg);
    fio.read((char*)&knob[i], sizeof(KnobConfig));
    knobID[i] = newID[i];
  }
  fio.close();
}

void Drambo::SetEncoderPtr(std::vector<KnobConfig> &knob, uint8_t offset)
{
  for(int i = 0; i < ENCODER_NUM; i++) 
  {
    Device::Encoder::Setup(&knob[offset + i], i);
    knobBar.SetPtr(&knob[offset + i],i);
  };
}

void Drambo::KnobBar_Toggle(uint8_t page) 
{
  std::vector<uint16_t> newID;
  for (uint8_t i = 0; i < ENCODER_NUM; i++) {
    uint8_t ch = MatrixOS::UserVar::global_MIDI_CH;
    uint8_t n = page * ENCODER_NUM + i;
    newID.push_back(ch << 8 | n);
  }

  Knob_SaveLoad(knobBar.knob, knobBar.knobID, newID);
  SetEncoderPtr(knobBar.knob);
  activeKnobPage = page;
}

bool Drambo::ConfigInit()
{
  for(uint8_t ch = 0; ch < 16; ch++){
    CH.color[ch] = COLOR_LIME;
    CH.type[ch] = 0;
  }
  CH.type[1]  = DRUM_PAD;
  CH.type[2]  = DRUM_PAD;
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

  PAD[0] = {.color = COLOR_AZURE, .rootColor = COLOR_BLUE, .type = PIANO_PAD, .scale = MAJOR};
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
    if (n < 8 ) CC[n].color = COLOR_YELLOW; else CC[n].color = COLOR_CYAN;
    CC[n].channel = n; 
    CC[n].byte1 = 9;
    CC[n].globalChannel = false;
  }

  Variable_Save(DRAMBO_ALL);
  return true;
}

bool Drambo::KnobInit()
{
  std::ofstream fout;
  fout.open(DRAMBO_KNOB_PATH, std::ios::binary);
  if (!fout.is_open()){
    MLOGD("FatFs", "Failed to init Knobs configuration.");
    return false;
  } // MLOGD("FatFs", "Succeeded to init  Knobs configuration.");

  uint8_t knobCC[DRAMBO_NUM_OF_KNOB] = {
    14,   15,   16,   17,   18,   19,   10,   7, 
    70,   71,   72,   73,   74,   75,   76,   77,
    90,   91,   92,   93,   94,   95,   96,   97,
    110,  111,  112,  113,  114,  115,  116,  117
  };
  
  for (uint8_t ch = 0; ch < 16; ch++) 
  {
    for (uint8_t n = 0; n < DRAMBO_NUM_OF_KNOB; n++) {
      KnobConfig tempKnob;
      tempKnob.channel = ch;
      tempKnob.byte1 = knobCC[n];
      tempKnob.enable = true;
      switch (n) { // set knob page 0 
        case 6: tempKnob.lock = true; tempKnob.byte2 = 63; tempKnob.def = 63; break;
        case 7: tempKnob.lock = true; tempKnob.byte2 = 78; tempKnob.def = 78; break;
      }
      switch (n / 8) {
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
