#include "MatrixOS.h"
#include "ui\UI.h"

namespace MatrixOS::Component {

    void Tab_ToggleSub(TabConfig* con) {
        if (con->subTab < con->subMax - 1){
            con->subTab++;
        } else con->subTab = 0;
    }

    void Channel_Setting(ChannelConfig* con, uint8_t n) {
        int8_t channel = n + 1;
        int8_t active = 0;
        Color activeColor = con->color[n];
        string name = "Channel " + std::to_string(channel) + " Setting";

        UI channelSetting(name, con->color[n]);

        UI4pxNumberWithColorFunc number([&]() -> Color { return activeColor; }, 3, &channel);

        UIButtonDimmable selectCC("CC for Select", Color(GREEN), [&]() -> bool { return active == 1; },
            [&]() -> void { active = 1; number.value = &con->selectCC; activeColor = Color(GREEN);});
        UIButtonDimmable muteCC("CC for Mute", Color(RED), [&]() -> bool { return active == 2; },
            [&]() -> void { active = 2; number.value = &con->muteCC; activeColor = Color(RED);});
        UIButtonDimmable soloCC("CC for Solo", Color(BLUE), [&]() -> bool { return active == 3; },
            [&]() -> void { active = 3;  number.value = &con->soloCC;activeColor = Color(BLUE);});
        
        UIButtonWithColorFunc plus_10("+10",[&]() -> Color { return (active != 0) ? Color(WHITE) : Color(BLANK);}, 
            [&]() -> void { if(active != 0) *number.value = ((int16_t)*number.value + 10 < 127) ? *number.value + 10 : 127;});
        UIButtonWithColorFunc plus_1("+1",[&]() -> Color { return (active != 0) ? Color(WHITE) : Color(BLANK);}, 
            [&]() -> void { if(active != 0) *number.value += (*number.value < 127); });
        UIButtonWithColorFunc minus_1("-1",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : Color(BLANK);}, 
            [&]() -> void { if(active != 0) *number.value -= (*number.value > 0); });
        UIButtonWithColorFunc minus_10("-10",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : Color(BLANK);}, 
            [&]() -> void { if(active != 0) *number.value = ((int16_t)*number.value - 10 > 0) ? *number.value - 10 : 0; });

        UIColorSelector colors(&con->color[n], [&]() -> void { active = 0;activeColor = con->color[n]; number.value = &channel;});

        channelSetting.AddUIComponent(number, Point(4, 0));

        channelSetting.AddUIComponent(selectCC, Point(5, 4));
        channelSetting.AddUIComponent(muteCC, Point(6, 4));
        channelSetting.AddUIComponent(soloCC, Point(7, 4));

        channelSetting.AddUIComponent(plus_10, Point(15, 0));
        channelSetting.AddUIComponent(plus_1, Point(15, 1));
        channelSetting.AddUIComponent(minus_1, Point(15, 2));
        channelSetting.AddUIComponent(minus_10, Point(15, 3));

        channelSetting.AddUIComponent(colors, Point(0, 0));

        channelSetting.Start();
        MatrixOS::FATFS::MarkChanged(con, 0);
    }

    void Knob_Setting(KnobConfig* con, bool channelSetting) {
        MatrixOS::LED::Fill(0);

        string byte1Name[4] = {"SYS", "CC", "PC Bank", "Note"};
        string byte2Name[4] = {"Value", "Value", "PC number", "Velocity"};
        Color typeColor[4] = {Color(BLANK), Color(YELLOW), Color(BLUE), Color(MAGENTA)};
        int8_t active;
        if(con->type == SEND_NONE)
            active = 3;
        else active = 2;

        UI knobSetting("Knob Setting", con->color);

        UI4pxNumberWithColorFunc numberChannel  ([&]() -> Color { return Color(GREEN); },             3, &con->data.channel, true);
        UI4pxNumberWithColorFunc numberByte1    ([&]() -> Color { return typeColor[con->type]; },   3, &con->data.byte1);
        UI4pxNumberWithColorFunc numberDef      ([&]() -> Color { return con->color; },             3, &con->def);
        UI4pxNumberWithColorFunc numberMin      ([&]() -> Color { return Color(CYAN); },            3, &con->min);
        UI4pxNumberWithColorFunc numberMax      ([&]() -> Color { return Color(RED); },              3, &con->max);

        UIButtonDimmable chSetting("Channel", Color(GREEN), [&]() -> bool { return active == 1; },
            [&]() -> void { active = 1; knobSetting.AddUIComponent(numberChannel, Point(4, 0));});
        UIButtonDimmable byte1Setting(byte1Name[con->type], typeColor[con->type], [&]() -> bool { return active == 2; },
            [&]() -> void { active = 2; knobSetting.AddUIComponent(numberByte1, Point(4, 0));});
        UIButtonDimmable defSetting("Default", con->color, [&]() -> bool { return active == 3;},
            [&]() -> void { active = 3; knobSetting.AddUIComponent(numberDef, Point(4, 0));});
        UIButtonDimmable minSetting("Min", Color(CYAN), [&]() -> bool { return active == 4;},
            [&]() -> void { active = 4; knobSetting.AddUIComponent(numberMin, Point(4, 0));});
        UIButtonDimmable maxSetting("Max", Color(RED), [&]() -> bool { return active == 5;},
            [&]() -> void { active = 5; knobSetting.AddUIComponent(numberMax, Point(4, 0));});

        // UIButtonDimmable sendCC("Send CC", Color(YELLOW), [&]() -> bool { return con->type == SEND_CC; },
        //     [&]() -> void { active = 2; number.value = &con->byte1, activeColor = Color(YELLOW); con->type = SEND_CC; 
        //                     byte1Setting.color = Color(YELLOW); byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});
        // UIButtonDimmable sendPC("Send PC", Color(BLUE), [&]() -> bool { return con->type == SEND_PC; },
        //     [&]() -> void { active = 2; number.value = &con->byte1, activeColor = Color(BLUE); con->type = SEND_PC; 
        //                     byte1Setting.color = Color(BLUE); byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});
        // UIButtonDimmable sendNote("Send Note", Color(MAGENTA), [&]() -> bool { return con->type == SEND_NOTE; },
        //     [&]() -> void { active = 2; number.value = &con->byte1, activeColor = Color(MAGENTA); con->type = SEND_NOTE; 
        //                     byte1Setting.color = Color(MAGENTA); byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});

        UIButtonWithColorFunc plus_10("+10",[&]() -> Color { return (active != 0) ? Color(WHITE) : Color(BLANK);}, 
            [&]() -> void { 
                switch(active) {
                    case 1: con->data.channel    = (con->data.channel + 10 < 15) ? con->data.channel + 10 : 15; break;  // channel
                    case 2: con->data.byte1      = (con->data.byte1 + 10 < 127) ? con->data.byte1 + 10 : 127; break; // cc / pc / note
                    case 3: con->def        = (con->def + 10 < con->max) ? con->def + 10 : con->max; break; // default
                    case 4: con->min        = (con->min + 10 < con->max) ? con->min + 10 : con->max; if(con->def < con->min) con->def = con->min; break; //min
                    case 5: con->max        = (con->max + 10 < 127) ? con->max + 10 : 127; //max
                }
            });
        UIButtonWithColorFunc plus_1("+1",[&]() -> Color { return (active != 0) ? Color(WHITE) : Color(BLANK);}, 
            [&]() -> void {
                switch(active) {
                    case 1: con->data.channel    = (con->data.channel + 1 < 15) ? con->data.channel + 1 : 15;break;  // channel
                    case 2: con->data.byte1      = (con->data.byte1 + 1 < 127) ? con->data.byte1 + 1 : 127; break; // cc / pc / note
                    case 3: con->def        = (con->def + 1 < con->max) ? con->def + 1 : con->max; break; // default
                    case 4: con->min        = (con->min + 1 < con->max) ? con->min + 1 : con->max; if(con->def < con->min) con->def = con->min; break; //min
                    case 5: con->max        = (con->max + 1 < 127) ? con->max + 1 : 127; break; //max
                }
            });
        UIButtonWithColorFunc minus_1("-1",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : Color(BLANK);}, 
            [&]() -> void {
                switch(active) {
                    case 1: con->data.channel    = (con->data.channel - 1 > 0) ? con->data.channel - 1 : 0; break; // channel
                    case 2: con->data.byte1      = (con->data.byte1 - 1 > 0) ? con->data.byte1 - 1 : 0; break; // cc / pc / note
                    case 3: con->def        = (con->def - 1 > con->min) ? con->def - 1 : con->min; break; // default
                    case 4: con->min        = (con->min - 1 > 0) ? con->min - 1 : 0; break; //min
                    case 5: con->max        = (con->max - 1 > con->min) ? con->max - 1 : con->min; if(con->def > con->max) con->def = con->max; break; //max
                }
            });
        UIButtonWithColorFunc minus_10("-10",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : Color(BLANK);}, 
            [&]() -> void {
                switch(active) {
                    case 1: con->data.channel    = (con->data.channel - 10 > 0) ? con->data.channel - 10 : 0; break; // channel
                    case 2: con->data.byte1      = (con->data.byte1 - 10 > 0) ? con->data.byte1 - 10 : 0; break; // cc / pc / note
                    case 3: con->def        = (con->def - 10 > con->min) ? con->def - 10 : con->min; break; // default
                    case 4: con->min        = (con->min - 10 > 0) ? con->min - 10 : 0; break; //min
                    case 5: con->max        = (con->max - 10 > con->min) ? con->max - 10 : con->min; if(con->def > con->max) con->def = con->max; break; //max
                }
             });

        UIColorSelector colors(&con->color, [&]() -> void {defSetting.color = con->color;});

        knobSetting.AddUIComponent(numberByte1, Point(4, 0));

        if(channelSetting) knobSetting.AddUIComponent(chSetting, Point(4, 4));
        knobSetting.AddUIComponent(byte1Setting, Point(4, 4));
        knobSetting.AddUIComponent(defSetting, Point(5, 4));
        knobSetting.AddUIComponent(minSetting, Point(6, 4));
        knobSetting.AddUIComponent(maxSetting, Point(7, 4));

        // knobSetting.AddUIComponent(sendCC, Point(0, 4));
        // knobSetting.AddUIComponent(sendPC, Point(1, 4));
        // knobSetting.AddUIComponent(sendNote, Point(2, 4));

        knobSetting.AddUIComponent(plus_10, Point(15, 0));
        knobSetting.AddUIComponent(plus_1, Point(15, 1));
        knobSetting.AddUIComponent(minus_1, Point(15, 2));
        knobSetting.AddUIComponent(minus_10, Point(15, 3));

        knobSetting.AddUIComponent(colors, Point(0, 0));

        knobSetting.Start();
        
        // end func
        MatrixOS::KnobCenter::MarkChanged(con->pos);
    }

    void Button_Setting(MidiButtonConfig* firstCon, uint16_t pos) {
        string byte1Name[4] = {"SYSTEM", "CC", "Bank", "Note"};
        string byte2Name[4] = {"Value", "Value", "PC", "Velocity"};
        Color typeColor[4] = {Color(BLANK), Color(YELLOW), Color(BLUE), Color(MAGENTA)};

        int8_t active = 2;
        string name = "Button Setting";
        Color chBtnColor = Color(GREEN);

        MidiButtonConfig* con = firstCon + pos;
        UI buttonSetting(name, con->color);

        UI4pxNumberWithColorFunc numberChannel  ([&]() -> Color { return chBtnColor.ToLowBrightness(!con->globalChannel); }, 3, &con->channel, true);
        UI4pxNumberWithColorFunc numberByte1    ([&]() -> Color { return typeColor[con->type]; }, 3, &con->byte1);
        UI4pxNumberWithColorFunc numberByte2    ([&]() -> Color { return con->color; }, 3, &con->byte2);

        UIButtonDimmable globalCH("Use Global Channel", Color(WHITE), [&]() -> bool { return con->globalChannel == true; },
            [&]() -> void { con->globalChannel = !con->globalChannel;});

        UIButtonDimmable chSetting("Channel", chBtnColor, [&]() -> bool { return active == 1; },
            [&]() -> void { active = 1; buttonSetting.AddUIComponent(numberChannel, Point(4, 0));});
        UIButtonDimmable byte1Setting(byte1Name[con->type], typeColor[con->type], [&]() -> bool { return active == 2; },
            [&]() -> void { active = 2; buttonSetting.AddUIComponent(numberByte1, Point(4, 0));});
        UIButtonDimmable byte2Setting(byte2Name[con->type], con->color, [&]() -> bool { return active == 3;},
            [&]() -> void { active = 3; buttonSetting.AddUIComponent(numberByte2, Point(4, 0));});

        UIButtonDimmable sendCC("Send CC", Color(YELLOW), [&]() -> bool { return con->type == SEND_CC; },
            [&]() -> void { active = 2; buttonSetting.AddUIComponent(numberByte1, Point(4, 0)); con->type = SEND_CC; 
                            byte1Setting.color = Color(YELLOW); byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});
        UIButtonDimmable sendPC("Send PC", Color(BLUE), [&]() -> bool { return con->type == SEND_PC; },
            [&]() -> void { active = 2; buttonSetting.AddUIComponent(numberByte1, Point(4, 0)); con->type = SEND_PC; 
                            byte1Setting.color = Color(BLUE); byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});
        UIButtonDimmable sendNote("Send Note", Color(MAGENTA), [&]() -> bool { return con->type == SEND_NOTE; },
            [&]() -> void { active = 2; buttonSetting.AddUIComponent(numberByte1, Point(4, 0)); con->type = SEND_NOTE; 
                            byte1Setting.color = Color(MAGENTA); byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});
        UIButtonDimmable toggle("Toggle Mode", Color(RED), [&]() -> bool { return con->toggleMode == true; },
            [&]() -> void { con->toggleMode = !con->toggleMode; });

        UIButtonWithColorFunc plus_10("+10",[&]() -> Color { return (active != 0) ? Color(WHITE) : Color(BLANK);}, 
            [&]() -> void { 
                switch(active) {
                    case 1: con->channel    = con->channel + 10 < 15    ? con->channel + 10     : 15;   break;
                    case 2: con->byte1      = con->byte1 + 10 < 127     ? con->byte1 + 10       : 127;  break;
                    case 3: con->byte2      = con->byte2 + 10 < 127     ? con->byte2 + 10       : 127;  break;
                }; });
        UIButtonWithColorFunc plus_1("+1",[&]() -> Color { return (active != 0) ? Color(WHITE) : Color(BLANK);}, 
            [&]() -> void { 
                switch(active) {
                    case 1: con->channel    += (con->channel < 15);     break;
                    case 2: con->byte1      += (con->byte1 < 127);      break;
                    case 3: con->byte2      += (con->byte2 < 127);      break;
                }; });
        UIButtonWithColorFunc minus_1("-1",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : Color(BLANK);}, 
            [&]() -> void { 
                switch(active) {
                    case 1: con->channel    -= (con->channel > 0);      break;
                    case 2: con->byte1      -= (con->byte1 > 0);        break;
                    case 3: con->byte2      -= (con->byte2 > 0);        break;
                }; });
        UIButtonWithColorFunc minus_10("-10",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : Color(BLANK);}, 
            [&]() -> void { 
                switch(active) {
                    case 1: con->channel    = con->channel - 10 > 0     ? con->channel - 10     : 0;    break;
                    case 2: con->byte1      = con->byte1 - 10 > 0       ? con->byte1 - 10       : 0;    break;
                    case 3: con->byte2      = con->byte2 - 10 > 0       ? con->byte2 - 10       : 0;    break;
                }; });

        UIColorSelector colors(&con->color, [&]() -> void {byte2Setting.color = con->color;});


        buttonSetting.AddUIComponent(numberByte1, Point(4, 0));

        buttonSetting.AddUIComponent(globalCH, Point(4, 4));
        buttonSetting.AddUIComponent(chSetting, Point(5, 4));
        buttonSetting.AddUIComponent(byte1Setting, Point(6, 4));
        buttonSetting.AddUIComponent(byte2Setting, Point(7, 4));

        buttonSetting.AddUIComponent(toggle, Point(3, 4));
        buttonSetting.AddUIComponent(sendCC, Point(0, 4));
        buttonSetting.AddUIComponent(sendPC, Point(1, 4));
        buttonSetting.AddUIComponent(sendNote, Point(2, 4));

        buttonSetting.AddUIComponent(plus_10, Point(15, 0));
        buttonSetting.AddUIComponent(plus_1, Point(15, 1));
        buttonSetting.AddUIComponent(minus_1, Point(15, 2));
        buttonSetting.AddUIComponent(minus_10, Point(15, 3));

        buttonSetting.AddUIComponent(colors, Point(0, 0));

        buttonSetting.Start();
        MatrixOS::FATFS::MarkChanged(firstCon, pos);
    }

    void DrumNote_Setting(MidiButtonConfig* firstCon, uint16_t pos) {
        MidiButtonConfig* con = firstCon + pos;
        UI drumNoteSetting("Note Setting", con->color);

        int8_t channel = MatrixOS::UserVar::global_channel;

        Color color3 = Color(CYAN);
        UI4pxNoteName noteName( &con->byte1, con->color, Color(WHITE), color3);

        UIButtonWithColorFunc plus_12("+Octave",[&]() -> Color { return color3;}, [&]() -> void {});
        plus_12.callback = [&]() -> void { 
            con->byte1 = (int)con->byte1 + 12 <= 127 ? con->byte1 + 12 : con->byte1;
            MatrixOS::MidiCenter::Hold(Point(15, 0), SEND_NOTE, channel, con->byte1); };

        UIButtonWithColorFunc plus_1("+Semi",[&]() -> Color { return con->color;}, [&]() -> void {});
        plus_1.callback = [&]() -> void { 
            con->byte1 += ((int)con->byte1 + 1 <= 127); 
            MatrixOS::MidiCenter::Hold(Point(15, 1), SEND_NOTE, channel, con->byte1); };

        UIButtonWithColorFunc minus_1("-Semi",[&]() -> Color { return con->color.ToLowBrightness();}, [&]() -> void {});
        minus_1.callback = [&]() -> void { 
            con->byte1 -= ((int)con->byte1 - 1 >= 0);
            MatrixOS::MidiCenter::Hold(Point(15, 2), SEND_NOTE, channel, con->byte1); };
        
        UIButtonWithColorFunc minus_12("-Octave",[&]() -> Color { return color3.ToLowBrightness();}, [&]() -> void {});
        minus_12.callback = [&]() -> void {
          con->byte1 = (int)con->byte1 - 12 >= 0 ? con->byte1 - 12 : con->byte1;
          MatrixOS::MidiCenter::Hold(Point(15, 3), SEND_NOTE, channel, con->byte1); };

        UIColorSelector colors(&(con->color), [&]() -> void { noteName.color1 = con->color; });

        drumNoteSetting.AddUIComponent(noteName, Point(6, 0));

        drumNoteSetting.AddUIComponent(plus_12, Point(15, 0));
        drumNoteSetting.AddUIComponent(plus_1, Point(15, 1));
        drumNoteSetting.AddUIComponent(minus_1, Point(15, 2));
        drumNoteSetting.AddUIComponent(minus_12, Point(15, 3));

        drumNoteSetting.AddUIComponent(colors, Point(0, 0));

        drumNoteSetting.Start();
        MatrixOS::FATFS::MarkChanged(firstCon, pos);
    }
    
    void Pad_Setting(NotePadConfig* firstCon, uint16_t pos, uint8_t padType){
        static uint16_t scales[32] = {  NATURAL_MINOR,
                                        MAJOR,
                                        DORIAN,
                                        PHRYGIAN,
                                        MIXOLYDIAN,
                                        MELODIC_MINOR_ASCENDING,
                                        HARMONIC_MINOR,
                                        BEBOP_DORIAN,
                                        BLUES,
                                        MINOR_PENTATONIC,
                                        HUNGARIAN_MINOR,
                                        UKRANIAN_DORIAN,
                                        MARVA,
                                        TODI,
                                        WHOLE_TONE,
                                        CHROMATIC,
                                        LYDIAN,
                                        LOCRIAN,
                                        MAJOR_PENTATONIC,
                                        PHYRIGIAN_DOMINATE,
                                        HALF_WHOLE_DIMINISHED,
                                        MIXOLYDIAN_BEBOP,
                                        SUPER_LOCRIAN,
                                        HIRAJOSHI,
                                        IN_SEN,
                                        YO_SCALE,
                                        IWATO,
                                        WHOLE_HALF,
                                        BEBOP_MINOR,
                                        MAJOR_BLUES,
                                        KUMOI,
                                        BEBOP_MAJOR };

        static string scale_names[32] =  {  "Natural Minor",
                                            "Major",
                                            "Dorian",
                                            "Phrygian",
                                            "Mixolydian",
                                            "Melodic Minor Ascending",
                                            "Harmonic Minor",
                                            "Bebop Dorian",
                                            "Blues",
                                            "Minor Pentatonic",
                                            "Hungarian Minor",
                                            "Ukranian Dorian",
                                            "Marva",
                                            "Todi",
                                            "Whole Tone",
                                            "Chromatic",
                                            "Lydian",
                                            "Locrian",
                                            "Major Pentatonic",
                                            "Phyrigian Dominate",
                                            "Half-Whole Diminished",
                                            "Mixolydian BeBop",
                                            "Super Locrian",
                                            "Hirajoshi",
                                            "In Sen",
                                            "Yo Scale",
                                            "Iwato",
                                            "Whole Half",
                                            "BeBop Minor",
                                            "Major Blues",
                                            "Kumoi",
                                            "BeBop Major" };

        Color color;
        if(padType == NOTE_PAD)
          color = COLOR_PIANO_PAD[1];
        else
          color = COLOR_NOTE_PAD[1];
        
        NotePadConfig* con = firstCon + pos;
        UI padSetting("Pad Setting", color);
        RootSelector rootSelector(Dimension(7, 2), con, 0, padType);
        UIItemSelector scaleSelector(Dimension(8, 4), Color(ORANGE), &con->scale, 32, scales, scale_names);
        // UISelector overlapSelector(Dimension(8, 1), "Overlap", Color(YELLOW), 8, &con->overlap);
        NotePreView notePreView(Dimension(8, 2), con);
        UISelector shiftSelector(Dimension(6, 1), "Shift", Color(BLUE), 6, &con->shift);
        UIButtonDimmable alignRootToggle("Aligh Root Key", Color(YELLOW), [&]() -> bool { return (con->alignRoot && con->overlap == 0); },
            [&]() -> void { if(con->overlap == 0) { con->alignRoot = !con->alignRoot;} else {con->alignRoot = true; con->overlap = 0;} });
        UIButtonDimmable enfourceScaleToggle("Enfource Scale", Color(ORANGE), [&]() -> bool { return con->enfourceScale; },
            [&]() -> void { con->enfourceScale = !con->enfourceScale; });

        padSetting.AddUIComponent(rootSelector, Point(0, 0));
        padSetting.AddUIComponent(scaleSelector, Point(8, 0));
        
        switch(padType){
            case NOTE_PAD:
                //padSetting.AddUIComponent(notePreView, Point(0, 2));
                padSetting.AddUIComponent(notePreView, Point(0, 2));
                padSetting.AddUIComponent(alignRootToggle, Point(6, 4));
                padSetting.AddUIComponent(enfourceScaleToggle, Point(7, 4));
                break;
            case PIANO_PAD: 
                padSetting.AddUIComponent(shiftSelector, Point(1, 3));
                break;
            case DRUM_PAD: break;
        }

        padSetting.Start();
        MatrixOS::FATFS::MarkChanged(firstCon, pos);
    }

    // void BPM_Setting() {
    //     UI bpmSetting("BPM Setting", Color(PURPLE));

    //     KnobConfig bpm = {.byte2 = MatrixOS::UserVar::BPM, .min = 20, .max = 300, .def = 120, .color = Color(PURPLE)};
    //     std::vector<KnobConfig*> knob = {&bpm};
    //     MatrixOS::KnobCenter::SetKnobBar(knob);

    //     UI4pxNumber numberBPM(Color(PURPLE), 3, &bpm.byte2);
    //     UIButtonWithColorFunc plus_10( "+10", 
    //         [&]() -> Color { return Color(WHITE); }, 
    //         [&]() -> void { bpm.byte2 = (bpm.byte2 + 10 < 300) ? bpm.byte2 + 10 : 300; });
    //     UIButtonWithColorFunc plus_1( "+1", 
    //         [&]() -> Color { return Color(WHITE); }, 
    //         [&]() -> void { bpm.byte2 += (bpm.byte2 < 300); });
    //     UIButtonWithColorFunc minus_1( "-1", 
    //         [&]() -> Color { return Color(0xFFFFFF).ToLowBrightness(); }, 
    //         [&]() -> void { bpm.byte2 -= (bpm.byte2 > 20); });
    //     UIButtonWithColorFunc minus_10( "-10", 
    //         [&]() -> Color { return Color(0xFFFFFF).ToLowBrightness(); }, 
    //         [&]() -> void { bpm.byte2 = (bpm.byte2 - 10 > 20) ? bpm.byte2 - 10 : 20; });

    //     bpmSetting.AddUIComponent(numberBPM, Point(4, 0));
    //     bpmSetting.AddUIComponent(plus_10, Point(15, 0));
    //     bpmSetting.AddUIComponent(plus_1, Point(15, 1));
    //     bpmSetting.AddUIComponent(minus_1, Point(15, 2));
    //     bpmSetting.AddUIComponent(minus_10, Point(15, 3));

    //     MatrixOS::KnobCenter::AddKnobBarTo(bpmSetting);
    //     bpmSetting.Start();
    //     MatrixOS::KnobCenter::DisableAll();
    //     MatrixOS::UserVar::BPM = bpm.byte2;
    // }

}
