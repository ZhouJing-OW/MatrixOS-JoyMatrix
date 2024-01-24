// #include "MainPad.h"
// #include "MainOctaveShifter.h"
// // #include "applications/Note/Scales.h"
// #include "ScaleVisualizer.h"
// #include "PianoPad.h"
// #include "NotePad.h"
// #include "KeySelector.h"
// #include "CCButton.h"
// #include "ChannelButton.h"
// #include "Shifter.h"
// #include "MixerKnobs.h"
// #include "Knobs.h"
// #include "MixerFaders.h"
// #include "MixerMatrix.h"


// #include "applications/BrightnessControl/BrightnessControl.h"

// void MainPad::Setup() {
//   // Set up / Load configs --------------------------------------------------------------------------

//   // Default Values
//   mainPadConfigs[1].color = Color(0xFF00FF);
//   mainPadConfigs[1].rootColor = Color(0x8800FF);
//   mainPadConfigs[1].scale = NATURAL_MINOR;
//   mainPadConfigs[1].overlap = 2;

//   // Default CCButtons
//   for (uint8_t i = 0; i < 2; i++){
//     for (uint8_t n = 0; n < 8; n++){
//       buttonConfigs[0].value2[i][n] = 127; 
//       buttonConfigs[0].value1[i][n] = i * 8 + n + 16;
//     }
//   }
//   for (uint8_t i = 2; i < 4; i++){
//     for (uint8_t n = 0; n < 8; n++){
//         buttonConfigs[0].type[i][n] = 2;
//         buttonConfigs[0].value2[i][n] = 127; 
//         buttonConfigs[0].value1[i][n] = i * 8 + 20 + n;
//         buttonConfigs[0].channel[i][n] = n + 1;
//     }
//   }
//   for (uint8_t i = 1; i < 16; i++){
//     if(i<9){
//       buttonConfigs[0].channelColor[i] = Color(0x0099FF);   
//     } else {
//       buttonConfigs[0].channelColor[i] = Color(0x4400FF);
//     }
//     buttonConfigs[0].channelSelect[i] = 120;
//     buttonConfigs[0].channelMute[i] = 119;
//     buttonConfigs[0].channelSolo[i] = 118;
//     buttonConfigs[0].channelCH[i] = i;
//   }
//   buttonConfigs[0].color[0] = {Color(0xFF9900)};
//   buttonConfigs[0].color[1] = {Color(0xFFFF00)};
//   buttonConfigs[0].color[2] = {Color(0x99FF00)};
//   buttonConfigs[0].color[3] = {Color(0x33FF00)};
//   buttonConfigs[0].activeGroup = 0;

//   // Default Knobs
//   for (uint8_t i = 0; i < 4; i++){
//     knobConfigs[0].value1[3][i + 4] = 7 + i;
//     knobConfigs[0].followChannelChange[i] = true;
//     for (uint8_t n = 0; n < 8; n++){
//       knobConfigs[0].value1[i][n] = 81 + i*10 + n;
//     }
//   }
//   for(uint8_t i = 0; i < 16; i++){
//     knobConfigs[0].value2[3][4][i] = 63;
//     knobConfigs[0].value2[3][7][i] = 77;
//   }
//   knobConfigs[0].value1[3][4] = 10;
//   knobConfigs[0].value1[3][5] = 8;
//   knobConfigs[0].value1[3][6] = 9;
//   knobConfigs[0].value1[3][7] = 7;
//   knobConfigs[0].value2[3][5][0] = 127;
//   knobConfigs[0].value2[3][6][0] = 127;
//   knobConfigs[0].defaultValue[3][4] = 64;
//   knobConfigs[0].defaultValue[3][7] = 77;
//   knobConfigs[0].color[0] = {Color(0x9900FF)};
//   knobConfigs[0].color[1] = {Color(0xFF00FF)};
//   knobConfigs[0].color[2] = {Color(0xFF0066)};
//   knobConfigs[0].color[3] = {Color(0xFF0000)};

//   // Load From NVS
//   if (nvsVersion == MAINPAD_APP_VERSION)
//   {
//     LoadVariable();
//   }
//   else
//   {
//     SaveVariable();
//   }

//   activeConfig.Get(); //Load it first
//   MatrixOS::NVS::SetVariable(MAINPAD_CONFIGS_HASH, mainPadConfigs, sizeof(mainPadConfigs));
//   // Set up the Action Menu UI ---------------------------------------------------------------------
//   UI actionMenu("Action Menu", Color(0x00FFFF));

//   uint8_t* activeButtonGroup = &buttonConfigs[0].activeGroup;
//   uint8_t* activeKnobGroup = &knobConfigs[0].activeGroup;
//   funcPage = 1;

//   actionMenu.SetLoopFunc([&]() -> void {
      
//     if(MatrixOS::SYS::FNExit == true) Exit();

//     switch (funcPage)
//     {
//       case 0:  // Do nothing
//       {
//         break;
//       }
//       case 1:  // Main Pad
//       {
//         funcPage = 0;
//         MainView();
//         break;
//       }
//       case 2:  // Note Pad
//       {
//         funcPage = 0;
//         Note();
//         break;
//       }
//       case 3:  //
//       {
//         funcPage = 0;
//         Mixer1();
//         break;
//       }
//       case 4:  //
//       {
//         funcPage = 0;
//         Mixer2();
//         break;
//       }
//       case 9:  //
//       {
//         funcPage = 0;
//         MatrixOS::SYS::OpenSetting();
//         break;
//       }
//     }
//   });

//   actionMenu.SetEndFunc([&]() -> void {      
//       KnobSetup(0);
//       SaveVariable();
//   });

//   // Main Pad Control

//   // KnobSetup(1);
//   // Knobs knobs(Dimension(8, 1), &knobConfigs[0], &knobConfigs[0].activeGroup, [&]() -> void {});
//   // actionMenu.AddUIComponent(knobs, Point(8, 4));

//   //--------------Function Page Selector--------------//

//   UIButtonWithColorFunc mainViewBtn("mainView", [&]() -> Color {return Color(0x60FF00).Scale(64); }, [&]() -> void {
//     MainPad::FuncPage(1);
//   }, [&]() -> void {});
//   actionMenu.AddUIComponent(mainViewBtn, Point(3, 4));

//   UIButtonWithColorFunc notePadBtn("NotePad", [&]() -> Color { return mainPadConfigs[lastActiveConfig2].color.Scale(64); }, [&]() -> void {
//     MainPad::FuncPage(2); 
//   }, [&]() -> void {});
//   actionMenu.AddUIComponent(notePadBtn, Point(4, 4));

//   UIButtonWithColorFunc Mixer("Mixer", [&]() -> Color { return Color(0xFFFF00).Scale(64); }, [&]() -> void {
//     MainPad::FuncPage(3); 
//   }, [&]() -> void {});
//   actionMenu.AddUIComponent(Mixer, Point(5, 4));

//   //--------------Function Page Selector--------------//

//   UIButtonWithColorFunc channelButton(
//       "Channel Selector Settings", [&]() -> Color { return buttonConfigs[0].channelColor[0]; }, [&]() -> void { ChannelBtnSettings(); });
//   actionMenu.AddUIComponent(channelButton, Point(0, 0));

//   UIButtonWithColorFunc buttonSettingBtn(
//       "Button Settings", [&]() -> Color { return buttonConfigs[0].color[*activeButtonGroup]; }, [&]() -> void { ButtonSettings(); });
//   actionMenu.AddUIComponent(buttonSettingBtn, Point(1, 0));

//   UIButtonWithColorFunc knobSettingBtn(
//       "Knob Settings", [&]() -> Color { return knobConfigs[0].color[*activeKnobGroup]; }, [&]() -> void { KnobSettings(); });
//   actionMenu.AddUIComponent(knobSettingBtn, Point(2, 0));

//   UIButtonWithColorFunc noteColorSelector(
//       "Note Color Selector", [&]() -> Color { return mainPadConfigs[activeConfig].color; }, [&]() -> void { ColorSelector(activeConfig); });
//   actionMenu.AddUIComponent(noteColorSelector, Point(0, 3));

//   UIButton velocitySettings("Velocity Settings", Color(0x0000FF), [&]() -> void { VelocitySettings(); });
//   actionMenu.AddUIComponent(velocitySettings, Point(0, 2));

//   UIButtonDimmable velocitySensitiveToggle(
//   "Velocity Sensitive", Color(0x00FFB0), [&]() -> bool { return MatrixOS::UserVar::pressureSensitive; },
//   [&]() -> void { MatrixOS::UserVar::pressureSensitive = !MatrixOS::UserVar::pressureSensitive; });
//   actionMenu.AddUIComponent(velocitySensitiveToggle, Point(1, 2));

//   UIButtonDimmable followChannelChange(
//       "Follow Channel Change", buttonConfigs[0].color[0], [&]() -> bool { return mainPadConfigs[activeConfig].followChannelChange; },
//       [&]() -> void { mainPadConfigs[activeConfig].followChannelChange = !mainPadConfigs[activeConfig].followChannelChange; });
//   actionMenu.AddUIComponent(followChannelChange, Point(1, 3));

//   UIButton scaleSelectorBtn("Scale Selector", Color(0xFF0090), [&]() -> void { ScaleSelector(); });
//   actionMenu.AddUIComponent(scaleSelectorBtn, Point(15, 2));

//   UIButtonWithColorFunc enforceScaleToggle(
//       "Enforce Scale", [&]() -> Color { 
//         if(activeConfig == 0 && lastPage == 1) return Color(0x000000);
//         else {
//           if(mainPadConfigs[activeConfig].enfourceScale)
//             return Color(0xff5000);
//           else
//             return Color(0xff5000).Scale(64);
//         }
//       },
//       [&]() -> void { 
//         if (!(activeConfig == 0 && lastPage == 1))
//           mainPadConfigs[activeConfig].enfourceScale = !mainPadConfigs[activeConfig].enfourceScale; 
//       });
//   actionMenu.AddUIComponent(enforceScaleToggle, Point(14, 2));

//   UIButtonWithColorFunc overlapSelectorBtn("Overlap Selector", [&]() -> Color { 
//         if(activeConfig == 0 && lastPage == 1) return Color(0x000000);
//         else return Color(0xFFFF00); 
//       }, [&]() -> void { if (!(activeConfig == 0 && lastPage == 1)) OverlapSelector(); });
//   actionMenu.AddUIComponent(overlapSelectorBtn, Point(14, 3));

//   UIButton channelSelectorBtn("Channel Selector", Color(0x60FF00), [&]() -> void { ChannelSelector(Color(0x60FF00), &mainPadConfigs[activeConfig].channel); });
//   actionMenu.AddUIComponent(channelSelectorBtn, Point(15, 3));

//   UIButtonWithColorFunc splitViewToggle("Split View", [&]() -> Color {         
//       if(lastPage == 2){
//         if(splitView == 1)
//           return Color(0xFFFFFF);
//         else
//           return Color(0xFFFFFF).Scale(64);
//         }
//         else return Color(0x000000); 
//       }, [&]() -> void { if (lastPage == 2) splitView = !splitView; });
//   actionMenu.AddUIComponent(splitViewToggle, Point(7, 1));
//   actionMenu.AddUIComponent(splitViewToggle, Point(8, 1));


//   // UIButtonDimmable velocitySensitiveToggle(
//   //     "Velocity Sensitive", Color(0x00FFB0), [&]() -> bool { return mainPadConfigs[activeConfig].velocitySensitive; },
//   //     [&]() -> void { mainPadConfigs[activeConfig].velocitySensitive = !mainPadConfigs[activeConfig].velocitySensitive; });
//   // actionMenu.AddUIComponent(velocitySensitiveToggle, Point(14, 0));

//   UIButtonWithColorFunc pianoKeyboardSelectBtn(
//       "Piano Keyboard", [&]() -> Color { return mainPadConfigs[0].color.ToLowBrightness(activeConfig.Get() == 0); },
//       [&]() -> void {
//         activeConfig = 0;
//         if (lastPage == 1)
//           lastActiveConfig1 = 0;
//         else if (lastPage == 2)
//           lastActiveConfig2 = 0;
//       });
//   actionMenu.AddUIComponent(pianoKeyboardSelectBtn, Point(7, 2));

//   UIButtonWithColorFunc notepadSelectBtn(
//       "Note Pad", [&]() -> Color { return mainPadConfigs[1].color.ToLowBrightness(activeConfig.Get() == 1); },
//       [&]() -> void {
//         activeConfig = 1;
//         if (lastPage == 1)
//           lastActiveConfig1 = 1;
//         else if (lastPage == 2)
//           lastActiveConfig2 = 1;
//       });
//   actionMenu.AddUIComponent(notepadSelectBtn, Point(8, 2));

//   MainOctaveShifter mainOctaveShifter( "Octave Shifter", 10 , mainPadConfigs, &activeConfig.value);
//   actionMenu.AddUIComponent(mainOctaveShifter, Point(3, 3));

//   // Other Controls
//   UIButton systemSettingBtn("System Setting", Color(0xFFFFFF), [&]() -> void { MatrixOS::SYS::OpenSetting(); });
//   actionMenu.AddUIComponent(systemSettingBtn, Point(15, 0));

//   actionMenu.SetKeyEventHandler([&](KeyEvent* keyEvent) -> bool {
//     if (keyEvent->id == FUNCTION_KEY)
//     { 
//       if (keyEvent->info.state == HOLD) {
//         MatrixOS::SYS::FNExit = 1;
//       }
//       else if (keyEvent->info.state == RELEASED)
//       {
//         SaveVariable();
//         MainPad::FuncPage(lastPage);
//       }
//       return true;  // Block UI from to do anything with FN, basiclly this function control the life cycle of the UI
//     }
//     return false;
//   });
//   actionMenu.AllowExit(false);
//   actionMenu.Start();

//   Exit();
// }


// void MainPad::Mixer1() {

//   MixerKnobSetup(0, 7);
//   bool active = 0;
//   bool knobstate = 0;

// void MainPad::VelocitySettings() {
//   UI velocitySettings ("Vlocity Settings", Color(0x0000FF));

//   UIButton defVelocity("Default Velocity", Color(0x0000FF), [&]() -> void {
//     uint8_t value = MatrixOS::UserVar::pressureToVelocity_Default;
//     value = MatrixOS::UIInterface::NumberSelector16x4(value, 0x0000FF, "Default Velocity", 0, 127);
//     MatrixOS::UserVar::pressureToVelocity_Default.Set(value);
//   });
//   velocitySettings.AddUIComponent(defVelocity, Point(0, 0));

//   UIButton minVelocity("Minimum Velocity", Color(0x00FF00), [&]() -> void {

//     uint8_t value = MatrixOS::UserVar::pressureToVelocity_Min;
//     value = MatrixOS::UIInterface::NumberSelector16x4(value, 0x00FF00, "Minimum Velocity", 0, 127);
//     MatrixOS::UserVar::pressureToVelocity_Min.Set(value);
//   });
//   velocitySettings.AddUIComponent(minVelocity, Point(1, 0));

//   UIButton maxVelocity("Maximum Velocity", Color(0xFF0000), [&]() -> void {
//     uint8_t value = MatrixOS::UserVar::pressureToVelocity_Max;
//     value = MatrixOS::UIInterface::NumberSelector16x4(value, 0xFF0000, "Maximum Velocity", 0, 127);
//     MatrixOS::UserVar::pressureToVelocity_Max.Set(value);
//   });
//   velocitySettings.AddUIComponent(maxVelocity, Point(2, 0));

//   velocitySettings.Start();

//   SaveVariable();
// }
