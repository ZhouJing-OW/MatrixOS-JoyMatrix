#include "Drambo.h"
#include "ui/component/PianoPad.h"
#include "ui/component/NotePad.h"



#include "applications/BrightnessControl/BrightnessControl.h"

void Drambo::Setup() {
  // Set up / Load configs --------------------------------------------------------------------------

  // Default Values
  config.Pad[0] = {
    .name = "Piano",
    .scale = CHROMATIC,
    .color = COLOR_AZURE,
    .rootColor = COLOR_BLUE,
  };

  config.Pad[1] = {
    .name = "Note Pad",
    .scale = NATURAL_MINOR,
    .overlap = 2,
    .color = COLOR_FUCHSIA,
    .rootColor = COLOR_VIOLET,
  };

  for (uint8_t ch = 0; ch < 16; ch++){
    config.CH[ch].channel = ch;

    for (uint8_t n = 0; n < 32; n++) {
      config.Knob[ch][n].value1 = n + 16;
      config.Knob[ch][n].SetSendCC();
      switch (n / 8) {
        case 0: config.Knob[ch][n].color = COLOR_RED; break;
        case 1: config.Knob[ch][n].color = COLOR_ORANGE; break;
        case 2: config.Knob[ch][n].color = COLOR_VIOLET; break;
        case 3: config.Knob[ch][n].color = COLOR_FUCHSIA; break;
      }
    }
    config.Knob[ch][4].value1 = 10;
    config.Knob[ch][4].value2 = 63;
    config.Knob[ch][4].def = 63;
    config.Knob[ch][5].value1 = 9;
    config.Knob[ch][6].value1 = 8;
    config.Knob[ch][7].value1 = 7;
    config.Knob[ch][7].value2 = 78;
    config.Knob[ch][7].def = 78;
  }

  config.Tab[0] = TabConfig{"Main", COLOR_LIME, 0, 2};
  config.Tab[1] = TabConfig{"Note", COLOR_FUCHSIA, 0, 2};
  config.Tab[2] = TabConfig{"Sequncer", COLOR_CYAN, 0, 1};
  config.Tab[3] = TabConfig{"Clips", COLOR_VIOLET, 0, 1};
  config.Tab[4] = TabConfig{"Mixer", COLOR_YELLOW, 0, 2};

  tabBar.Setup(config.Tab, 5, activeTab, toggle);
  knobBar.Setup(config.Knob[MatrixOS::UserVar::global_MIDI_CH], 8);

  // Load From NVS
  if (nvsVersion == DRAMBO_APP_VERSION)
  {
    LoadVariable();
  }
  else
  {
    SaveVariable();
  }

  UI setting("Setting", COLOR_BLUE);
  setting.SetLoopFunc([&]() -> void {
      
    if(MatrixOS::SYS::FNExit == true) Exit();

    switch (toggle)
    {
      case -1: break;
      case 0: toggle = -1; activeTab = 0; Tab0(); break;
      case 1: toggle = -1; activeTab = 1; Tab1(); break;
      case 2: toggle = -1; activeTab = 2; Tab2(); break;
      case 3: toggle = -1; activeTab = 3; Tab3(); break;
      case 4: toggle = -1; activeTab = 4; Tab4(); break;
      case 9: toggle = -1; activeTab = 9; Pop();  break;
    }

  });

  setting.SetEndFunc([&]() -> void {
      Device::Encoder::DisableAll();
      SaveVariable();
  });

  setting.SetKeyEventHandler([&](KeyEvent* keyEvent) -> bool {
    if (keyEvent->id == FUNCTION_KEY)
    { 
      if (keyEvent->info.state == HOLD) {
        MatrixOS::SYS::FNExit = true;
      }
      else if (keyEvent->info.state == RELEASED)
      {
        SaveVariable();
        toggle = activeTab;
      }
      return true;  // Block UI from to do anything with FN, basiclly this function control the life cycle of the UI
    }
    return false;
  });

  setting.AllowExit(false);
  setting.Start();

  Exit();
}



void Drambo::Tab0(){ // Main
  for (int i = 0; i < 8; i++){
    Device::Encoder::Setup(config.Knob[MatrixOS::UserVar::global_MIDI_CH] + i, i);
  }
  
  UI tab0("");
  PianoPad piano(Dimension(16, 2), 0, &config.Pad[0]);
  NotePad note(Dimension(16, 2), &config.Pad[1]);
  
  switch (config.Tab[0].subTab){
    case 0:
      tab0.AddUIComponent(piano, Point(0, 2));
      break;
    case 1:
      tab0.AddUIComponent(note, Point(0, 2));
      break;
  }

  tab0.AddUIComponent(mainBar, Point(0, 4));
  tab0.AddUIComponent(tabBar, Point(3, 4));
  tab0.AddUIComponent(knobBar, Point(8, 4));
  tab0.SetLoopFunc([&]() -> void { if (toggle >= 0) tab0.Exit(); });
  tab0.Start();
  Device::Encoder::DisableAll();
}

void Drambo::Tab1(){ // Note
  UI tab1("");

  tab1.AddUIComponent(mainBar, Point(0, 4));
  tab1.AddUIComponent(tabBar, Point(3, 4));
  tab1.AddUIComponent(knobBar, Point(8, 4));
  tab1.SetLoopFunc([&]() -> void { if (toggle >= 0) tab1.Exit(); });
  tab1.Start();
  Device::Encoder::DisableAll();
}

void Drambo::Tab2(){ // Sequncer
  UI tab2("");

  tab2.AddUIComponent(mainBar, Point(0, 4));
  tab2.AddUIComponent(tabBar, Point(3, 4));
  tab2.AddUIComponent(knobBar, Point(8, 4));
  tab2.SetLoopFunc([&]() -> void { if (toggle >= 0) tab2.Exit(); });
  tab2.Start();
  Device::Encoder::DisableAll();
}

void Drambo::Tab3(){ // Clips
  UI tab3("");

  tab3.AddUIComponent(mainBar, Point(0, 4));
  tab3.AddUIComponent(tabBar, Point(3, 4));
  tab3.AddUIComponent(knobBar, Point(8, 4));
  tab3.SetLoopFunc([&]() -> void { if (toggle >= 0) tab3.Exit(); });
  tab3.Start();
  Device::Encoder::DisableAll();
}

void Drambo::Tab4(){ // Mixer
  UI tab4("");

  tab4.AddUIComponent(mainBar, Point(0, 4));
  tab4.AddUIComponent(tabBar, Point(3, 4));
  tab4.AddUIComponent(knobBar, Point(8, 4));
  tab4.SetLoopFunc([&]() -> void { if (toggle >= 0) tab4.Exit(); });
  tab4.Start();
  Device::Encoder::DisableAll();
}

void Drambo::Pop(){
  UI pop("");

  pop.AddUIComponent(knobBar, Point(8, 4));
  pop.Start();
}

void Drambo::LoadVariable(){
  MatrixOS::NVS::GetVariable(DRAMBO_CONFIGS_HASH, &config, sizeof(config));
}

void Drambo::SaveVariable(){
  MatrixOS::NVS::SetVariable(DRAMBO_CONFIGS_HASH, &config, sizeof(config));
}

