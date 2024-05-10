#include "Drambo.h"

void Drambo::Setup() {
  if(!Inited || nvsVersion != DRAMBO_APP_VERSION) { // Set up configs
    bool configInit = ConfigInit();
    bool knobInit = KnobInit();
    if (configInit && knobInit) Inited = true;
  }
  
  MatrixOS::FATFS::VarManager(info.name, CONFIG_SUFFIX, saveVarList);
  MatrixOS::KnobCenter::RequestService(info.name);
  MatrixOS::MidiCenter::RequestService(info.name, CH);
  
  transBar.Setup(MatrixOS::MidiCenter::TransState());
  tabBar.Setup(TAB, 5, activeTab, toggle);

  volumeMixer.Setup(Dimension(16, 4), 16, true, true);
  mixer.Setup(Dimension(16, 4), 64, true); 
  volumeMixerPos.reserve(16);
  mixerPos.reserve(64);
  for (uint8_t ch = 0; ch < 16; ch++) volumeMixerPos.push_back(ch * DRAMBO_NUM_OF_KNOB);
  for (uint8_t n = 0; n < 4; n++)
    for (uint8_t ch = 0; ch < 16; ch++)
      mixerPos.push_back(ch * DRAMBO_NUM_OF_KNOB + n);

  UI setup("", Color(BLUE));
  setup.SetLoopFunc([&]() -> void {
    if(MatrixOS::SYS::FNExit == true) Exit();
    toggleTab();
  });
  setup.AllowExit(false);
  setup.Start();
}

void Drambo::TabS(){
  UI tabS("Setting", Color(BLUE));
  Device::KeyPad::ClearPad();
  std::vector<KnobConfig*> sysKnobs = MatrixOS::MidiCenter::GetSysKnobs();
  MatrixOS::KnobCenter::SetKnobBar(sysKnobs);

  UI4pxKnobVar number(sysKnobs, 3);
  // UIButtonLarge systemSettingBtn("System Setting", Color(WHITE), Dimension(2,2), [&]() -> void { MatrixOS::SYS::OpenSetting(); });
  UIButtonLarge tap("Tap to BPM", Color(BLUE), Dimension(2, 1), [&]() -> void { });
  UIButtonLarge exitBtn("Exit", Color(RED), Dimension(2, 2), [&]() -> void { ExitApp();});
  UIButtonDimmable knob1(
      "bpm", sysKnobs[0]->color, [&]() -> bool { return number.activeKnob == 0; }, [&]() -> void { number.activeKnob = 0; });
  UIButtonDimmable knob2(
      "swing", sysKnobs[1]->color, [&]() -> bool { return number.activeKnob == 1; }, [&]() -> void { number.activeKnob = 1; });
  UIButtonDimmable knob3(
      "Default velocity", sysKnobs[2]->color, [&]() -> bool { return number.activeKnob == 2; }, [&]() -> void { number.activeKnob = 2; });
  UIButtonDimmable knob8(
      "Brightness", sysKnobs[3]->color, [&]() -> bool { return number.activeKnob == 3; }, [&]() -> void { number.activeKnob = 3; });
    
  UIButtonDimmable bluetooth(
    "Bluetooth", Color(BLUE), []() -> bool { return Device::BLEMIDI::started; },
    []() -> void { Device::BLEMIDI::Toggle(); Device::bluetooth = Device::BLEMIDI::started; });
  UIButtonDimmable clockOut(
      "Clock Out", sysKnobs[0]->color, [&]() -> bool { return MatrixOS::UserVar::clockOut; },
      [&]() -> void {
        MatrixOS::UserVar::clockOut = !MatrixOS::UserVar::clockOut;
        MatrixOS::UserVar::clockIn = false;
      });
  UIButtonDimmable clockIn(
      "Clock In", sysKnobs[0]->color, [&]() -> bool { return MatrixOS::UserVar::clockIn; },
      [&]() -> void {
        MatrixOS::UserVar::clockIn = !MatrixOS::UserVar::clockIn;
        MatrixOS::UserVar::clockOut = false;
      });

  tabS.AddUIComponent(tap, Point(0, 2));
  tabS.AddUIComponent(number, Point(3, 0));
  tabS.AddUIComponent(exitBtn, Point(0, 0));
  tabS.AddUIComponent(knob1, Point(0, 4));
  tabS.AddUIComponent(knob2, Point(1, 4));
  tabS.AddUIComponent(knob3, Point(2, 4));
  tabS.AddUIComponent(knob8, Point(3, 4));
  tabS.AddUIComponent(bluetooth, Point(7, 4));
  tabS.AddUIComponent(clockOut, Point(0, 3));
  tabS.AddUIComponent(clockIn, Point(1, 3));

  MatrixOS::KnobCenter::AddKnobBarTo(tabS);
  tabS.SetLoopFunc([&]() -> void { CommonLoop(tabS); });
  tabS.SetEndFunc([&]() -> void { CommonEnd(tabS); });
  tabS.Start();

  // exit Function
  toggle = activeTab;
}

void Drambo::Tab0(){ // Main
  UI tab0("");
  MatrixOS::KnobCenter::ChannelMode();
  MatrixOS::MidiCenter::AddMidiAppTo(tab0);

  // int8_t num;
  CommonUI(tab0);
  // tab0.SetScrollBar(&num, 4, COLOR_SEQ_4PAGE);
  tab0.SetLoopFunc([&]() -> void { CommonLoop(tab0); });
  tab0.SetEndFunc([&]() -> void { CommonEnd(tab0); });
  tab0.Start();
}

void Drambo::Tab1(){ // Note
  UI tab1("");
  MatrixOS::KnobCenter::ChannelMode();
  MatrixOS::MidiCenter::AddClipSelectorTo(tab1);

  CommonUI(tab1);
  tab1.SetLoopFunc([&]() -> void { CommonLoop(tab1); });
  tab1.SetEndFunc([&]() -> void { CommonEnd(tab1); });
  tab1.Start();
}

void Drambo::Tab2(){ // Sequncer
  UI tab2("");

  CommonUI(tab2);
  tab2.SetLoopFunc([&]() -> void { CommonLoop(tab2); });
  tab2.SetEndFunc([&]() -> void { CommonEnd(tab2); });
  tab2.Start();
}

void Drambo::Tab3(){ // Clips 
  UI tab3("");

  CommonUI(tab3);
  tab3.SetLoopFunc([&]() -> void { CommonLoop(tab3); });
  tab3.SetEndFunc([&]() -> void { CommonEnd(tab3); });
  tab3.Start();
}

void Drambo::Tab4(){ // Mixer
  UI tab4("");
  int8_t x = 0;
  int8_t y = 3;
  
  if (TAB[4].subTab == 0)
  {
    volumeMixer.SetKnobs(volumeMixerPos, 0);
    tab4.AddUIComponent(volumeMixer, Point(0, 0));
    Device::AnalogInput::SetLeftRight(&x, 1, 0, 1, true, [&]() -> void { volumeMixer.SetActivePoint(x * 8); });
  }
  else 
  {
    mixer.SetKnobs(mixerPos, 0);
    tab4.AddUIComponent(mixer, Point(0, 0));
    Device::AnalogInput::SetLeftRight(&x, 1, 0, 1, true,  [&]() -> void { mixer.SetActivePoint(x * 8 + y * 16); });
    Device::AnalogInput::SetUpDown(&y, 3, 0, 1, true, [&]() -> void { mixer.SetActivePoint(x * 8 + y * 16); });
  }

  CommonUI(tab4);
  tab4.SetLoopFunc( [&]() -> void { CommonLoop(tab4); });
  tab4.SetEndFunc([&]() -> void { Device::AnalogInput::DisableUpDown(); CommonEnd(tab4); });
  tab4.Start();
}

void Drambo::Pop(){
  UI pop("");
  Device::KeyPad::ClearPad();
  uint8_t chn = MatrixOS::UserVar::global_channel;

  UIPlusMinus ProgramBank(&CH->bankLSB[chn], 127, 1, Color(TURQUOISE), false, false,
    [&]() -> void { MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, chn, 0, CH->bankLSB[chn])); });
  UIPlusMinus ProgramNum(&CH->PC[chn], 127, 0, Color(CYAN), false, false,
    [&]()-> void { MatrixOS::MIDI::Send(MidiPacket(0, ProgramChange, chn, CH->PC[chn], 0));});
  ChannelButton channel(Dimension(16, 1), CH, MatrixOS::MidiCenter::TransState(), true, [&]() -> void { 
    chn = MatrixOS::UserVar::global_channel;
    ProgramBank.val = &CH->bankLSB[chn];
    ProgramNum.val = &CH->PC[chn];});
  MidiButton ccBtn(Dimension(8, 1), CC, 16);

  pop.AddUIComponent(channel, Point(0, 0));
  pop.AddUIComponent(ProgramBank, Point(0, 1));
  pop.AddUIComponent(ProgramNum, Point(14, 1));
  pop.AddUIComponent(ccBtn, Point(4, 1));

  MatrixOS::KnobCenter::AddKnobBarTo(pop);
  pop.SetLoopFunc([&]() -> void { if(Device::KeyPad::AltExit()) { Device::KeyPad::ClearPad(); pop.Exit();} });
  pop.Start();
}

void Drambo::toggleTab()
{
  Device::AnalogInput::DisableDirectPad();
  MatrixOS::KnobCenter::DisableAll();
  switch (toggle)
  {
    case -1: TabS(); break;
    case 0: toggle = -1; activeTab = 0; Tab0(); break;
    case 1: toggle = -1; activeTab = 1; Tab1(); break;
    case 2: toggle = -1; activeTab = 2; Tab2(); break;
    case 3: toggle = -1; activeTab = 3; Tab3(); break;
    case 4: toggle = -1; activeTab = 4; Tab4(); break;
  }
}

void Drambo::CommonUI(UI &ui)
{
  ui.AddUIComponent(transBar, Point(0, 4));
  ui.AddUIComponent(tabBar, Point(3, 4));
  MatrixOS::KnobCenter::AddKnobBarTo(ui);
}

void Drambo::CommonLoop(UI &ui)
{
  if (Device::KeyPad::LAltState.state == PRESSED || Device::KeyPad::RAltState.state == PRESSED) { Pop(); } 
  if (toggle >= 0) { ui.Exit(); }
}

void Drambo::CommonEnd(UI &ui)
{
  uint8_t thisLayer = MatrixOS::LED::CurrentLayer(); 
  MatrixOS::LED::CopyLayer(thisLayer - 1, thisLayer);
}

bool Drambo::ConfigInit()
{
  TAB = new TabConfig[5];
  TAB[0] = TabConfig{"MAIN", Color(GREEN), 0, 3};
  TAB[1] = TabConfig{"NOTE", Color(PURPLE), 0, 2};
  TAB[2] = TabConfig{"SEQ", Color(TURQUOISE), 0, 1};
  TAB[3] = TabConfig{"CLIP", Color(VIOLET), 0, 1};
  TAB[4] = TabConfig{"MIXER", Color(YELLOW),0, 2};

  CC = new MidiButtonConfig[16];
  for (uint8_t n = 0; n < 16; n++) {
    if (n < 8 ) CC[n].color = Color(YELLOW); else CC[n].color = Color(TURQUOISE);
    CC[n].channel = n; 
    CC[n].byte1 = 9;
    CC[n].globalChannel = false;
  }

  PT = new MidiButtonConfig[16];

  bool rtn = MatrixOS::FATFS::ListSave(info.name, CONFIG_SUFFIX, saveVarList, true);

  // delete CH; delete[] PAD; delete[] DRUM;
  delete[] TAB; delete[] CC; delete[] PT;

  return rtn;
}

bool Drambo::KnobInit()
{
  if(MatrixOS::KnobCenter::OpenFile(info.name))
  {
    uint8_t knobCC[DRAMBO_NUM_OF_KNOB] = {
      7,    10,   14,   15,   16,   17,   18,   19, 
      70,   71,   72,   73,   74,   75,   76,   77,
      90,   91,   92,   93,   94,   95,   96,   97,
      110,  111,  112,  113,  114,  115,  116,  117
    };
    
    for (uint8_t ch = 0; ch < 16; ch++) 
    {
      for (uint8_t n = 0; n < DRAMBO_NUM_OF_KNOB; n++) {
        KnobConfig tempKnob;
        tempKnob.data.channel = ch;
        tempKnob.type = SEND_CC;
        tempKnob.data.byte1 = knobCC[n];
        tempKnob.color = COLOR_KNOB_8PAGE[n / 8];
        switch (n)
        {  // set knob page 0
          case 0: tempKnob.lock = true; tempKnob.data.byte2 = 80; tempKnob.def = 80; break;
          case 1: tempKnob.lock = true; tempKnob.middleMode = true; tempKnob.data.byte2 = 63; tempKnob.def = 63; break;
        }
        MatrixOS::KnobCenter::SaveKnobContinuous(tempKnob);
      }
    }
    MatrixOS::KnobCenter::CloseFile();
    return true;
  }
  return false;
}

void Drambo::ExitApp()
{
  MatrixOS::FATFS::VarManageEnd(CONFIG_SUFFIX);
  MatrixOS::KnobCenter::EndService();
  MatrixOS::MidiCenter::EndService();
  Device::AnalogInput::DisableDirectPad();
  MatrixOS::SYS::FNExit = true;
}