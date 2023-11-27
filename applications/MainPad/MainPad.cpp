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

// void MainPad::MainView() {

//   UI mainView("");
//   mainView.SetSetupFunc([&]() -> void { KnobSetup(1); });
//   mainView.SetEndFunc([&]() -> void { KnobSetup(0); });
//   uint8_t* activeButtonGroup = &buttonConfigs[0].activeGroup;
//   uint8_t* activeKnobGroup = &knobConfigs[0].activeGroup;
//   activeConfig = lastActiveConfig1;

//   Knobs knobs(Dimension(8, 1), &knobConfigs[0], &knobConfigs[0].activeGroup, [&]() -> void {});
//   mainView.AddUIComponent(knobs, Point(8, 4));


//   //--------------Function Page Selector--------------//

//   UIButtonWithColorFunc mainViewBtn("mainView", [&]() -> Color { return Color(0x60FF00); }, [&]() -> void {
//   }, [&]() -> void {});
//   mainView.AddUIComponent(mainViewBtn, Point(3, 4));

//   UIButtonWithColorFunc notePadBtn("NotePad", [&]() -> Color { return mainPadConfigs[lastActiveConfig2].color.Scale(64); }, [&]() -> void {
//     MainPad::FuncPage(2); 
//     mainView.Exit();
//   }, [&]() -> void {});
//   mainView.AddUIComponent(notePadBtn, Point(4, 4));

//   UIButtonWithColorFunc Mixer("Mixer", [&]() -> Color { return Color(0xFFFF00).Scale(64); }, [&]() -> void {
//     MainPad::FuncPage(3); 
//     mainView.Exit();
//   }, [&]() -> void {});
//   mainView.AddUIComponent(Mixer, Point(5, 4));

//   //--------------Function Page Selector--------------//

//   // 16 Channel Selector Buttons group 0
//   ChannelButton channelButton(Dimension(16, 1), 64, &buttonConfigs[0], [&]() -> void {
//       for (uint8_t i = 0; i < 2; i++)
//       {
//         mainPadConfigs[i].channel = buttonConfigs[0].activeChannel;
//       };

//       for (uint8_t n = 0; n < 4; n++)
//       {
//         if (knobConfigs[0].followChannelChange[n] == true)
//         {
//           for (uint8_t m = 0; m < 8; m++)
//           {
//             knobConfigs[0].channel[n][m] = buttonConfigs[0].activeChannel;
//           }
//           KnobSetup(1);
//         }
//       }
//   });
//   mainView.AddUIComponent(channelButton, Point(0, 0));

//   // 8x4 CC Buttons group 1-4
//   CCButton ccButton(Dimension(8, 1), 255, &buttonConfigs[0], activeButtonGroup, [&]() -> void {});
//   mainView.AddUIComponent(ccButton, Point(4, 1));

//   //Button Selector
//   UIButtonWithColorFunc Button1(
//       "Button Group 1", [&]() -> Color { return *activeButtonGroup == 0 ? Color(0xFFFFFF) : buttonConfigs[0].color[0]; },
//       [&]() -> void {buttonConfigs[0].activeGroup = 0;},
//       [&]() -> void {buttonConfigs[0].activeGroup = 0;}
//       );
//   mainView.AddUIComponent(Button1, Point(0, 1));

//   UIButtonWithColorFunc Button2(
//       "Button Group 2", [&]() -> Color { return  *activeButtonGroup == 1 ? Color(0xFFFFFF) : buttonConfigs[0].color[1]; },
//       [&]() -> void {buttonConfigs[0].activeGroup = 1;},
//       [&]() -> void {buttonConfigs[0].activeGroup = 1;}
//       );
//   mainView.AddUIComponent(Button2, Point(1, 1));

//   UIButtonWithColorFunc Button3(
//       "Button Group 3", [&]() -> Color { return  *activeButtonGroup == 2 ? Color(0xFFFFFF) : buttonConfigs[0].color[2]; },
//       [&]() -> void {buttonConfigs[0].activeGroup = 2;},
//       [&]() -> void {buttonConfigs[0].activeGroup = 2;}
//       );
//   mainView.AddUIComponent(Button3, Point(2, 1));

//   UIButtonWithColorFunc Button4(
//       "Button Group 4", [&]() -> Color { return  *activeButtonGroup == 3 ? Color(0xFFFFFF) : buttonConfigs[0].color[3]; },
//       [&]() -> void {buttonConfigs[0].activeGroup = 3;},
//       [&]() -> void {buttonConfigs[0].activeGroup = 3;}
//       );
//   mainView.AddUIComponent(Button4, Point(3, 1));

//   // Knob Selector
//   UIButtonWithColorFunc Knob1(
//       "Knob Group 1", [&]() -> Color { return *activeKnobGroup == 0 ? Color(0xFFFFFF) : knobConfigs[0].color[0]; },
//       [&]() -> void {knobConfigs[0].activeGroup = 0; KnobSetup(1);},
//       [&]() -> void {knobConfigs[0].activeGroup = 0; KnobSetup(1);}
//       );
//   mainView.AddUIComponent(Knob1, Point(12, 1));

//   UIButtonWithColorFunc Knob2(
//       "Knob Group 2", [&]() -> Color { return *activeKnobGroup == 1 ? Color(0xFFFFFF) : knobConfigs[0].color[1]; },
//       [&]() -> void {knobConfigs[0].activeGroup = 1; KnobSetup(1);},
//       [&]() -> void {knobConfigs[0].activeGroup = 1; KnobSetup(1);}
//       );
//   mainView.AddUIComponent(Knob2, Point(13, 1));

//   UIButtonWithColorFunc Knob3(
//       "Knob Group 3", [&]() -> Color { return *activeKnobGroup == 2 ? Color(0xFFFFFF) : knobConfigs[0].color[2]; },
//       [&]() -> void {knobConfigs[0].activeGroup = 2; KnobSetup(1);},
//       [&]() -> void {knobConfigs[0].activeGroup = 2; KnobSetup(1);}
//       );
//   mainView.AddUIComponent(Knob3, Point(14, 1));

//   UIButtonWithColorFunc Knob4(
//       "Knob Group 4", [&]() -> Color { return *activeKnobGroup == 3 ? Color(0xFFFFFF) : knobConfigs[0].color[3]; },
//       [&]() -> void {knobConfigs[0].activeGroup = 3; KnobSetup(1);},
//       [&]() -> void {knobConfigs[0].activeGroup = 3; KnobSetup(1);}
//       );
//   mainView.AddUIComponent(Knob4, Point(15, 1));

//   PianoPad pianoPad1(0, &mainPadConfigs[activeConfig]);
//   PianoPad pianoPad2(1, &mainPadConfigs[activeConfig]);
//   PianoPad pianoPad3(2, &mainPadConfigs[activeConfig]);
//   if (activeConfig == 0)
//   {
//     mainView.AddUIComponent(pianoPad1, Point(1, 2));
//     mainView.AddUIComponent(pianoPad2, Point(8, 2));
//     mainView.AddUIComponent(pianoPad3, Point(15, 2));
//   };

//   MainNotePad notePad(Dimension(15, 2), &mainPadConfigs[1]);
//   if(activeConfig == 1){
//     mainView.AddUIComponent(notePad, Point(1, 2));
//   };

//   UIButtonWithColorFunc octaveUp(
//     "Octave Up", [&]() -> Color { return Color(0xFFFFFF).Scale(mainPadConfigs[activeConfig].octave * 25 + 30); },
//     [&]() -> void {
//       uint8_t activeNotes = notePad.activeNotes.size() + pianoPad1.activeNotes.size() + pianoPad2.activeNotes.size() + pianoPad3.activeNotes.size();
//       if ((!activeNotes) && (mainPadConfigs[activeConfig].octave < 9))
//       {
//         mainPadConfigs[activeConfig].octave++;
//         notePad.GenerateKeymap();
//       };
//     },
//     []() -> void {});
//   mainView.AddUIComponent(octaveUp, Point(0, 2));

//   UIButtonWithColorFunc octaveDown(
//     "Octave Down", [&]() -> Color { return Color(0xFFFFFF).Scale(255 - (mainPadConfigs[activeConfig].octave * 25)); },
//     [&]() -> void {
//       uint8_t activeNotes = notePad.activeNotes.size() + pianoPad1.activeNotes.size() + pianoPad2.activeNotes.size() + pianoPad3.activeNotes.size();
//       if ((!activeNotes) && (mainPadConfigs[activeConfig].octave > 0))
//       {
//         mainPadConfigs[activeConfig].octave--;
//         notePad.GenerateKeymap();
//       };
//     },
//     []() -> void {});
//   mainView.AddUIComponent(octaveDown, Point(0, 3));

//   mainView.Start();
// }

// void MainPad::Note(){

//   UI note("");

//   note.SetSetupFunc([&]() -> void { KnobSetup(1); });
//   note.SetEndFunc([&]() -> void { KnobSetup(0); });
//   activeConfig = lastActiveConfig2;

//   //--------------Function Page Selector--------------//

//   UIButtonWithColorFunc mainViewBtn("mainView", [&]() -> Color { return Color(0x60FF00).Scale(64); }, [&]() -> void {
//     MainPad::FuncPage(1); 
//     note.Exit();
//   }, [&]() -> void {});
//   note.AddUIComponent(mainViewBtn, Point(3, 4));

//   UIButtonWithColorFunc notePadBtn("NotePad", [&]() -> Color { return mainPadConfigs[lastActiveConfig2].color; },[&]() -> void {
//     splitView = !splitView;
//     MainPad::FuncPage(2);
//     note.Exit();
//   },[&]() -> void {});
//   note.AddUIComponent(notePadBtn, Point(4, 4));

//   UIButtonWithColorFunc Mixer("Mixer", [&]() -> Color { return Color(0xFFFF00).Scale(64); }, [&]() -> void {
//     MainPad::FuncPage(3); 
//     note.Exit();
//   }, [&]() -> void {});
//   note.AddUIComponent(Mixer, Point(5, 4));

//   //--------------Function Page Selector--------------//

//   Knobs knobs(Dimension(8, 1), &knobConfigs[0], &knobConfigs[0].activeGroup, [&]() -> void {});
//   note.AddUIComponent(knobs, Point(8, 4));

//   uint8_t group3 = 3;
//   uint8_t group2 = 2;
//   CCButton Button1(Dimension(4, 2), 255, &buttonConfigs[0], &group3, [&]() -> void {});
//   CCButton Button2(Dimension(4, 2), 255, &buttonConfigs[0], &group2, [&]() -> void {});
//   if (splitView){
//   note.AddUIComponent(Button1, Point(0, 0));
//   note.AddUIComponent(Button1, Point(12, 0));
//   note.AddUIComponent(Button2, Point(0, 2));
//   note.AddUIComponent(Button2, Point(12, 2));
//   }

//   MainNotePad notePad1(Dimension(splitView ? 8 : 16, 4), &mainPadConfigs[activeConfig]);
//   note.AddUIComponent(notePad1, splitView ? Point(4, 0) : Point(0, 0));

//   // MainNotePad notePad2(Dimension(8, 4), &mainPadConfigs[1]);
//   // if (splitView)
//   // { note.AddUIComponent(notePad2, Point(8, 0)); }

//   note.Start();
// }

// void MainPad::Mixer1() {

//   MixerKnobSetup(0, 7);
//   bool active = 0;
//   bool knobstate = 0;

//   UI Mixer1("");

//     //--------------Function Page Selector--------------//

//   UIButtonWithColorFunc mainViewBtn("mainView", [&]() -> Color { return Color(0x60FF00).Scale(64); }, [&]() -> void {
//     MainPad::FuncPage(1); 
//     Mixer1.Exit();
//   }, [&]() -> void {});
//   Mixer1.AddUIComponent(mainViewBtn, Point(3, 4));

//   UIButtonWithColorFunc notePadBtn("NotePad", [&]() -> Color { return mainPadConfigs[lastActiveConfig2].color.Scale(64); }, [&]() -> void {
//     MainPad::FuncPage(2); 
//     Mixer1.Exit();
//   }, [&]() -> void {});
//   Mixer1.AddUIComponent(notePadBtn, Point(4, 4));

//   UIButtonWithColorFunc Mixer("Mixer", [&]() -> Color { return Color(0xFFFF00); }, [&]() -> void {
//     MainPad::FuncPage(4); 
//     Mixer1.Exit();
//   }, [&]() -> void {});
//   Mixer1.AddUIComponent(Mixer, Point(5, 4));

//   //--------------Function Page Selector--------------//

//   MixerFaders faders(Dimension(16,4), &buttonConfigs[0], &knobConfigs[0], &active , [&]() -> void {
//     if (knobstate != active){
//       if (active == 0 ) {
//         MixerKnobSetup(0, 7);
//         knobstate = active;
//       }
//       if (active == 1 ) {
//         MixerKnobSetup(1, 7);
//         knobstate = active;
//       }      
//     }
//   });
//   Mixer1.AddUIComponent(faders, Point(0, 0));

//   MixerKnobs knobBar([&]() -> void {});
//   Mixer1.AddUIComponent(knobBar, Point(8, 4));

//   Mixer1.Start();
//   SaveVariable();

// }

// void MainPad::Mixer2() {

//   bool active = 0;
//   uint8_t number = 7;

//   UI Mixer2("");

//   //--------------Function Page Selector--------------//

//   UIButtonWithColorFunc mainViewBtn("mainView", [&]() -> Color { return Color(0x60FF00).Scale(64); }, [&]() -> void {
//     MainPad::FuncPage(1); 
//     Mixer2.Exit();
//   }, [&]() -> void {});
//   Mixer2.AddUIComponent(mainViewBtn, Point(3, 4));

//   UIButtonWithColorFunc notePadBtn("NotePad", [&]() -> Color { return mainPadConfigs[lastActiveConfig2].color.Scale(64); }, [&]() -> void {
//     MainPad::FuncPage(2); 
//     Mixer2.Exit();
//   }, [&]() -> void {});
//   Mixer2.AddUIComponent(notePadBtn, Point(4, 4));

//   UIButtonWithColorFunc Mixer("Mixer", [&]() -> Color { return Color(0xFF9900); }, [&]() -> void {
//     MainPad::FuncPage(3); 
//     Mixer2.Exit();
//   }, [&]() -> void {});
//   Mixer2.AddUIComponent(Mixer, Point(5, 4));

//   //--------------Function Page Selector--------------//

//   MixerMatrix mixerMatrix(&buttonConfigs[0], &knobConfigs[0], &number, &active, [&]() -> void {MainPad::MixerKnobSetup(active, number);});
//   Mixer2.AddUIComponent(mixerMatrix, Point(0, 0));

//   MixerKnobs knobBar([&]() -> void {});
//   Mixer2.AddUIComponent(knobBar, Point(8, 4));

//   Mixer2.Start();
//   SaveVariable();


// }

// void MainPad::ButtonSettings() {

//   uint8_t* activeButtonGroup = &buttonConfigs[0].activeGroup;

//   UI ButtonSettings("Button Settings", Color(0x00FF00));

//   UIButtonWithColorFunc button1(
//       "Group 1 Settings", [&]() -> Color { return *activeButtonGroup == 0 ? Color(0xFFFFFF) : buttonConfigs[0].color[0]; }, [&]() -> void { *activeButtonGroup = 0; });
//   ButtonSettings.AddUIComponent(button1, Point(6, 3));
//   UIButtonWithColorFunc button2(
//       "Group 2 Settings", [&]() -> Color { return *activeButtonGroup == 1 ? Color(0xFFFFFF) : buttonConfigs[0].color[1]; }, [&]() -> void { *activeButtonGroup = 1; });
//   ButtonSettings.AddUIComponent(button2, Point(7, 3));
//   UIButtonWithColorFunc button3(
//       "Group 3 Settings", [&]() -> Color { return *activeButtonGroup == 2 ? Color(0xFFFFFF) : buttonConfigs[0].color[2]; }, [&]() -> void { *activeButtonGroup = 2; });
//   ButtonSettings.AddUIComponent(button3, Point(8, 3));
//   UIButtonWithColorFunc button4(
//       "Group 4 Settings", [&]() -> Color { return *activeButtonGroup == 3 ? Color(0xFFFFFF) : buttonConfigs[0].color[3]; }, [&]() -> void { *activeButtonGroup = 3; });
//   ButtonSettings.AddUIComponent(button4, Point(9, 3));
  

//   CCButton ccButton(Dimension(8, 1), 64, &buttonConfigs[0], activeButtonGroup, []() -> void {});
//   ButtonSettings.AddUIComponent(ccButton, Point(4, 1));

//   uint8_t* activeButton = &ccButton.activeButton;

//   UIButtonLargeWithColorFunc ButtonColor(
//       "Color", [&]() -> Color { return buttonConfigs[0].color[*activeButtonGroup]; }, Dimension(1, 1),
//       [&]() -> void {
//         MatrixOS::UIInterface::ColorPicker(buttonConfigs[0].color[*activeButtonGroup]);
//         MatrixOS::NVS::SetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//       });
//   ButtonSettings.AddUIComponent(ButtonColor, Point(0, 3));

//   UIButton ccChannel("Channel", Color(0x60FF00), [&]() -> void { 
//       ChannelSelector(Color(0x60FF00), &buttonConfigs[0].channel[*activeButtonGroup][*activeButton]);
//       MatrixOS::NVS::SetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//       });
//   ButtonSettings.AddUIComponent(ccChannel, Point(15, 3));

//   UIButtonWithColorFunc commandChangeBtn(
//       "Type = CC", [&]() -> Color { return Color (0x6000FF).ToLowBrightness(buttonConfigs[0].type[*activeButtonGroup][*activeButton] == 0); }, [&]() -> void { 
//         buttonConfigs[0].type[*activeButtonGroup][*activeButton] = 0; 
//         MatrixOS::NVS::SetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//       });
//   ButtonSettings.AddUIComponent(commandChangeBtn, Point(13, 2));

//   UIButtonWithColorFunc programChangeBtn(
//       "Type = PC", [&]() -> Color { return Color (0xFF00FF).ToLowBrightness(buttonConfigs[0].type[*activeButtonGroup][*activeButton] == 1); }, [&]() -> void { 
//         buttonConfigs[0].type[*activeButtonGroup][*activeButton] = 1;
//         MatrixOS::NVS::SetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//       });
//   ButtonSettings.AddUIComponent(programChangeBtn, Point(14, 2));

//   UIButtonWithColorFunc NoteBtn(
//       "Type = Note", [&]() -> Color { return Color (0xFF0060).ToLowBrightness(buttonConfigs[0].type[*activeButtonGroup][*activeButton] == 2); }, [&]() -> void { 
//         buttonConfigs[0].type[*activeButtonGroup][*activeButton] = 2;
//         MatrixOS::NVS::SetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//       });
//   ButtonSettings.AddUIComponent(NoteBtn, Point(15, 2));

//   UIButton ccNumber("Number", Color(0xFF6000), [&]() -> void {
//     if(buttonConfigs[0].type[*activeButtonGroup][*activeButton] == 2){
//       NoteSelector(*activeButtonGroup, *activeButton);
//     }
//     else
//     {
//       uint8_t* value1 = &buttonConfigs[0].value1[*activeButtonGroup][*activeButton];
//       *value1 = MatrixOS::UIInterface::NumberSelector16x4(*value1, 0xFF6000, "Number", 0, 127);
//     }
//   });
//   ButtonSettings.AddUIComponent(ccNumber, Point(13, 3));

//   UIButton value("Value", Color(0x00FF60), [&]() -> void {
//     uint8_t* value2 = &buttonConfigs[0].value2[*activeButtonGroup][*activeButton];
//     *value2 = MatrixOS::UIInterface::NumberSelector16x4(*value2, 0x00FF60, "Value", 0, 127);
//   });
//   ButtonSettings.AddUIComponent(value, Point(14, 3));

//   ButtonSettings.Start();

//   SaveVariable();
// }

// void MainPad::ChannelBtnSettings(){

//   uint8_t* activeChannel = &buttonConfigs[0].activeChannel;

//   UI ChannelBtnSettings("Channel Button Settings", Color(0x00FF00));


//   ChannelButton channelButton(Dimension(16, 1), 64, &buttonConfigs[0], []() -> void {});
//   ChannelBtnSettings.AddUIComponent(channelButton, Point(0, 0));

//   UIButtonLargeWithColorFunc ButtonColor(
//       "Color", [&]() -> Color { return buttonConfigs[0].channelColor[*activeChannel]; }, Dimension(1, 1),
//       [&]() -> void {
//         MatrixOS::UIInterface::ColorPicker(buttonConfigs[0].channelColor[*activeChannel]);
//         MatrixOS::NVS::SetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//       });
//   ChannelBtnSettings.AddUIComponent(ButtonColor, Point(0, 3));

//   UIButton channel("Channel", Color(0x60FF00), [&]() -> void { 
//     ChannelSelector(Color(0x60FF00),&buttonConfigs[0].channelCH[*activeChannel]);
//     MatrixOS::NVS::SetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//   });
//   ChannelBtnSettings.AddUIComponent(channel, Point(15, 3));

//   UIButton selectCC("Select CC", Color(0xFF6000), [&]() -> void {
//     uint8_t* value1 = &buttonConfigs[0].channelSelect[*activeChannel];
//     *value1 = MatrixOS::UIInterface::NumberSelector16x4(*value1, 0xFF6000, "Select CC", 0, 127);
//     MatrixOS::NVS::SetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//   });
//   ChannelBtnSettings.AddUIComponent(selectCC, Point(12, 3));

//   UIButton muteCC("Mute CC", Color(0xFF0000), [&]() -> void {
//     uint8_t* value1 = &buttonConfigs[0].channelMute[*activeChannel];
//     *value1 = MatrixOS::UIInterface::NumberSelector16x4(*value1, 0xFF0000, "Mute CC", 0, 127);
//     MatrixOS::NVS::SetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//   });
//   ChannelBtnSettings.AddUIComponent(muteCC, Point(13, 3));

//   UIButton soloCC("Solo CC", Color(0x00FFFF), [&]() -> void {
//     uint8_t* value1 = &buttonConfigs[0].channelSolo[*activeChannel];
//     *value1 = MatrixOS::UIInterface::NumberSelector16x4(*value1, 0x00FFFF, "Solo CC", 0, 127);
//     MatrixOS::NVS::SetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//   });
//   ChannelBtnSettings.AddUIComponent(soloCC, Point(14, 3));

//   ChannelBtnSettings.Start();

//   SaveVariable();

// }

// void MainPad::KnobSettings(){

//   uint8_t* activeKnobGroup = &knobConfigs[0].activeGroup;
//   uint8_t* activeKnob = Device::Encoder::GetActiveKnob();

//   UI KnobSettings("Knob Settings", Color(0x00FF00));

//   KnobSetup(0);
//   Color knobColor = knobConfigs[0].color[*activeKnobGroup];
//   Color knobColorLow = knobConfigs[0].color[*activeKnobGroup].Scale(64);

//   Shifter activeKnobList(Dimension(8, 1), "Active Knob", &knobColorLow, &knobColor, activeKnob);
//   KnobSettings.AddUIComponent(activeKnobList,(Point(8, 4)));

//   UIButtonWithColorFunc knob1(
//       "Group 1", [&]() -> Color { return *activeKnobGroup == 0 ? Color(0xFFFFFF) : knobConfigs[0].color[0]; }, 
//       [&]() -> void { 
//         *activeKnobGroup = 0; 
//         knobColor = knobConfigs[0].color[*activeKnobGroup];
//         knobColorLow = knobConfigs[0].color[*activeKnobGroup].Scale(64);
//         });
//   KnobSettings.AddUIComponent(knob1, Point(6, 3));

//   UIButtonWithColorFunc knob2(
//       "Group 2", [&]() -> Color { return *activeKnobGroup == 1 ? Color(0xFFFFFF) : knobConfigs[0].color[1]; }, 
//       [&]() -> void { 
//         *activeKnobGroup = 1; 
//         knobColor = knobConfigs[0].color[*activeKnobGroup];
//         knobColorLow = knobConfigs[0].color[*activeKnobGroup].Scale(64);
//         });
//   KnobSettings.AddUIComponent(knob2, Point(7, 3));

//   UIButtonWithColorFunc knob3(
//       "Group 3", [&]() -> Color { return *activeKnobGroup == 2 ? Color(0xFFFFFF) : knobConfigs[0].color[2]; }, 
//       [&]() -> void { 
//         *activeKnobGroup = 2; 
//         knobColor = knobConfigs[0].color[*activeKnobGroup];
//         knobColorLow = knobConfigs[0].color[*activeKnobGroup].Scale(64);        
//         });
//   KnobSettings.AddUIComponent(knob3, Point(8, 3));

//   UIButtonWithColorFunc knob4(
//       "Group 4", [&]() -> Color { return *activeKnobGroup == 3 ? Color(0xFFFFFF) : knobConfigs[0].color[3]; }, 
//       [&]() -> void { 
//         *activeKnobGroup = 3; 
//         knobColor = knobConfigs[0].color[*activeKnobGroup];
//         knobColorLow = knobConfigs[0].color[*activeKnobGroup].Scale(64);        
//         });
//   KnobSettings.AddUIComponent(knob4, Point(9, 3));

//   // Knobs knobs(Dimension(8, 1), &knobConfigs[0], &group, []() -> void {}); 
//   // KnobSettings.AddUIComponent(knobs,(Point(8, 4)));

//   UIButtonLargeWithColorFunc colorBtn(
//       "Color", [&]() -> Color { return knobConfigs[0].color[*activeKnobGroup]; }, Dimension(1, 1),
//       [&]() -> void {
//         MatrixOS::UIInterface::ColorPicker(knobConfigs[0].color[*activeKnobGroup]);
//         MatrixOS::NVS::SetVariable(KNOB_CONFIGS_HASH, knobConfigs, sizeof(knobConfigs));
//       });
//   KnobSettings.AddUIComponent(colorBtn, Point(0, 3));

//   UIButton knobChannel("Channel", Color(0x60FF00), [&]() -> void { 
//       ChannelSelector(Color(0x60FF00),&knobConfigs[0].channel[*activeKnobGroup][*activeKnob]);
//       MatrixOS::NVS::SetVariable(KNOB_CONFIGS_HASH, knobConfigs, sizeof(knobConfigs));
//       });
//   KnobSettings.AddUIComponent(knobChannel, Point(15, 3));

//   UIButtonDimmable followChannelChange(
//       "Follow Channel Change", buttonConfigs[0].color[0], [&]() -> bool { return knobConfigs[0].followChannelChange[*activeKnobGroup]; },
//       [&]() -> void { knobConfigs[0].followChannelChange[*activeKnobGroup] = !knobConfigs[0].followChannelChange[*activeKnobGroup]; });
//   KnobSettings.AddUIComponent(followChannelChange, Point(1, 3));

//   UIButton ccNumber("CC Number", Color(0xFF6000), [&]() -> void {
//     uint8_t* value1 = knobConfigs[0].value1[*activeKnobGroup] + *activeKnob;
//     *value1 = MatrixOS::UIInterface::NumberSelector16x4(*value1, 0xFF6000, "CC", 0, 127);
//   });
//   KnobSettings.AddUIComponent(ccNumber, Point(13, 3));

//   UIButton defaultValue("Default Value", Color(0x00FF60), [&]() -> void {
//     int32_t* defaultValue = knobConfigs[0].defaultValue[*activeKnobGroup] + *activeKnob;
//     *defaultValue = MatrixOS::UIInterface::NumberSelector16x4(*defaultValue, 0x00FF60, "Default Value", 0, 127);
//   });
//   KnobSettings.AddUIComponent(defaultValue, Point(14, 3));

//   KnobSettings.Start();

//   SaveVariable();
// }

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

// void MainPad::ScaleSelector() {
//   UI scaleSelector("Scale Selector", Color(0xFF0090));

//   ScaleVisualizer scaleVisualizer(&mainPadConfigs[activeConfig].rootKey, &mainPadConfigs[activeConfig].scale, mainPadConfigs[activeConfig].color,
//                                   mainPadConfigs[activeConfig].rootColor);
//   scaleSelector.AddUIComponent(scaleVisualizer, Point(0, 2));

//   UIItemSelector scaleSelectorBar(Dimension(8, 4), Color(0xFF0090), &mainPadConfigs[activeConfig].scale, 32, scales, scale_names);
//   scaleSelector.AddUIComponent(scaleSelectorBar, Point(8, 0));

//   scaleSelector.Start();

//   SaveVariable();
// }

// void MainPad::OverlapSelector() {
//   UI overlapSelector("Overlap Selector", Color(0xFFFF00));

//   UI4pxNumber numDisplay(Color(0xFFFF00), 2, (int32_t*)&mainPadConfigs[activeConfig].overlap, mainPadConfigs[activeConfig].rootColor);
//   overlapSelector.AddUIComponent(numDisplay, Point(9, 0));

//   UISelector overlapInput(Dimension(8, 2), "Overlap", Color(0xFFFF00), 15, (uint16_t*)&mainPadConfigs[activeConfig].overlap);
//   overlapSelector.AddUIComponent(overlapInput, Point(0, 2));

//   UIButtonDimmable alignRootToggle(
//       "Aligh Root Key", Color(0xFFFFFF), [&]() -> bool { return mainPadConfigs[activeConfig].alignRoot; },
//       [&]() -> void { mainPadConfigs[activeConfig].alignRoot = !mainPadConfigs[activeConfig].alignRoot; });
//   overlapSelector.AddUIComponent(alignRootToggle, Point(0, 0));

//   overlapSelector.Start();

//   SaveVariable();
// }

// void MainPad::ColorSelector(uint8_t con) {
//   UI colorSelector("Color Selector", mainPadConfigs[con].color);

//   PianoPad pianoPad1(1,&mainPadConfigs[con]);
//   colorSelector.AddUIComponent(pianoPad1, Point(0, 1));

//   UIButtonLargeWithColorFunc rootColorSelectorBtn(
//       "Root Key Color", [&]() -> Color { return mainPadConfigs[con].rootColor; }, Dimension(2, 2),
//       [&]() -> void { MatrixOS::UIInterface::ColorPicker(mainPadConfigs[con].rootColor); });
//   colorSelector.AddUIComponent(rootColorSelectorBtn, Point(9, 1));

//   UIButtonLargeWithColorFunc pianoPadColorSelectorBtn(
//       "Keyboard Color", [&]() -> Color { return mainPadConfigs[con].color; }, Dimension(2, 2),
//       [&]() -> void { MatrixOS::UIInterface::ColorPicker(mainPadConfigs[con].color); });
//   colorSelector.AddUIComponent(pianoPadColorSelectorBtn, Point(13, 1));

//   colorSelector.Start();

//   SaveVariable();
// }


// void MainPad::ChannelSelector(Color color, uint8_t* channel) {
//   UI channelSelector("Channel Selector", color);
//   uint16_t CH;
//   int32_t offsettedChannel = *channel + 1;

//   UI4pxNumber numDisplay(color, 2, &offsettedChannel, color, 1);
//   channelSelector.AddUIComponent(numDisplay, Point(9, 0));

//   UISelector channelInput(Dimension(8, 2), "Channel", color, 16, &CH, [&](uint16_t val) -> void {
//     offsettedChannel = val + 1;
//     *channel = CH & 0xFF;
//   });
//   channelSelector.AddUIComponent(channelInput, Point(0, 2));

//   channelSelector.Start();

//   SaveVariable();
// }

// void MainPad::NoteSelector(uint8_t group, uint8_t number){
//   UI noteSelector("Note Selector", buttonConfigs[0].color[group]);
//   uint8_t octave = buttonConfigs[0].value1[group][number] / 12;

//   KeySelector keySelector(&octave, group, number, &buttonConfigs[0]);
//   noteSelector.AddUIComponent(keySelector, Point(1, 1));

//   Shifter octaveSelector(Dimension(6, 2), "octave", &mainPadConfigs[activeConfig].color, &mainPadConfigs[activeConfig].rootColor, &octave, 11);
//   noteSelector.AddUIComponent(octaveSelector,(Point(9, 1)));

//   noteSelector.Start();

//   SaveVariable();

// }

// void MainPad::KnobSetup(uint8_t type){
//   uint8_t AKG = knobConfigs[0].activeGroup;
//   uint8_t ch;
//   uint8_t val1;
//   int32_t* Ptr;
//   int32_t defaultValue;
//   Color* color = &knobConfigs[0].color[AKG];

//   for (uint8_t n = 0; n < 8; n++){
//     defaultValue = knobConfigs[0].defaultValue[AKG][n];
//     ch = knobConfigs[0].channel[AKG][n];
//     val1 = knobConfigs[0].value1[AKG][n];
//     Ptr = &knobConfigs[0].value2[AKG][n][ch];
//     Device::Encoder::Setup(color, type, ch, val1, Ptr, 0, 127, defaultValue, n);
//   }
// }

// void MainPad::MixerKnobSetup(bool active, uint8_t i){
//   int32_t* Ptr;
//   int32_t defaultValue = knobConfigs[0].defaultValue[3][i];
//   uint8_t ch;
//   uint8_t val1 = knobConfigs[0].value1[3][i];
//   uint8_t n = 0;
//   Color* color;
//   bool activeState[16];

//   for (uint8_t i = 0; i < 16; i++) {
//     activeState[i] = (i >= buttonConfigs[0].channelGroupDivide && i < buttonConfigs[0].channelGroupDivide + 8) ? 1 : 0;
//   }

//   for (uint8_t x = 0; x < 16; x++)
//   {
//     if (activeState[x] == active)
//     {
//       color = &buttonConfigs[0].channelColor[x];
//       ch = x;
//       Ptr = &knobConfigs[0].value2[3][i][x];
//       Device::Encoder::Setup(color, 1, ch, val1, Ptr, 0, 127, defaultValue, n);
//       n++;
//     }
//   }
// }

// void MainPad::FuncPage(uint8_t page){
//   funcPage = page;
//   lastPage = page;
// }

// void MainPad::LoadVariable(){
//   MatrixOS::NVS::GetVariable(MAINPAD_CONFIGS_HASH, mainPadConfigs, sizeof(mainPadConfigs));
//   MatrixOS::NVS::GetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//   MatrixOS::NVS::GetVariable(KNOB_CONFIGS_HASH, knobConfigs, sizeof(knobConfigs));
// }

// void MainPad::SaveVariable(){
//   MatrixOS::NVS::SetVariable(MAINPAD_CONFIGS_HASH, mainPadConfigs, sizeof(mainPadConfigs));
//   MatrixOS::NVS::SetVariable(CC_CONFIGS_HASH, buttonConfigs, sizeof(buttonConfigs));
//   MatrixOS::NVS::SetVariable(KNOB_CONFIGS_HASH, knobConfigs, sizeof(knobConfigs));
// }

