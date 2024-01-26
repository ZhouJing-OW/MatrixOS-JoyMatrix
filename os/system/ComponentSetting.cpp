#include "MatrixOS.h"
#include "ui\UI.h"

namespace MatrixOS::Component {

    void Tab_ToggleSub(TabConfig* con) {
        if (con->subTab < con->subMax - 1){
            con->subTab++;
        } else con->subTab = 0;
    }

    void Knob_Function(KnobConfig* con) {
        con->changed = true;
        switch (con->type) {
            case SEND_NONE: break;
            case SEND_CC: 
                MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, con->channel, con->byte1, con->byte2)); 
                break;
            case SEND_PC: 
                MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, con->channel, 0, con->byte1));
                MatrixOS::MIDI::Send(MidiPacket(0, ProgramChange, con->channel, con->byte2, con->byte2)); 
                break;
        }
    }

    void Channel_Setting(ChannelConfig* con, uint8_t n) {
        MatrixOS::KEYPAD::Clear();
        MatrixOS::LED::Fill(0);

        int8_t channel = n + 1;
        int8_t active = 0;
        Color activeColor = con->color[n];
        string name = "Channel " + std::to_string(channel) + " Setting";

        UI channelSetting(name, con->color[n]);

        UI4pxNumberWithColorFunc number([&]() -> Color { return activeColor; }, 3, &channel);

        UIButtonDimmable selectCC("CC for Select", COLOR_LIME, [&]() -> bool { return active == 1; },
            [&]() -> void { active = 1; number.value = &con->selectCC; activeColor = COLOR_LIME;});
        UIButtonDimmable muteCC("CC for Mute", COLOR_RED, [&]() -> bool { return active == 2; },
            [&]() -> void { active = 2; number.value = &con->muteCC; activeColor = COLOR_RED;});
        UIButtonDimmable soloCC("CC for Solo", COLOR_BLUE, [&]() -> bool { return active == 3; },
            [&]() -> void { active = 3;  number.value = &con->soloCC;activeColor = COLOR_BLUE;});
        
        UIButtonWithColorFunc plus_10("+10",[&]() -> Color { return (active != 0) ? COLOR_WHITE : COLOR_BLANK;}, 
            [&]() -> void { if(active != 0) *number.value = ((int16_t)*number.value + 10 < 127) ? *number.value + 10 : 127;});
        UIButtonWithColorFunc plus_1("+1",[&]() -> Color { return (active != 0) ? COLOR_WHITE : COLOR_BLANK;}, 
            [&]() -> void { if(active != 0) *number.value += (*number.value < 127); });
        UIButtonWithColorFunc minus_1("-1",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : COLOR_BLANK;}, 
            [&]() -> void { if(active != 0) *number.value -= (*number.value > 0); });
        UIButtonWithColorFunc minus_10("-10",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : COLOR_BLANK;}, 
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
    }

    void Knob_Setting(KnobConfig* con, bool channelSetting) {
        MatrixOS::KEYPAD::Clear();
        MatrixOS::LED::Fill(0);
        con->changed = true;

        string byte1Name[4] = {"SYS", "CC", "PC Bank", "Note"};
        string byte2Name[4] = {"Value", "Value", "PC number", "Velocity"};
        Color typeColor[4] = {COLOR_BLANK, COLOR_YELLOW, COLOR_BLUE, COLOR_PINK};

        int8_t active = 2;
        int8_t* activeVal = &(con->byte1);
        Color activeColor = typeColor[con->type];

        UI knobSetting("Knob Setting", con->color);

        UI4pxNumberWithColorFunc number([&]() -> Color { return activeColor; }, 3, activeVal);

        UIButtonDimmable chSetting("Channel", COLOR_LIME, [&]() -> bool { return active == 1; },
            [&]() -> void { active = 1; number.value = &con->channel; activeColor = COLOR_LIME; number.add1 = true;});
        UIButtonDimmable byte1Setting(byte1Name[con->type], typeColor[con->type], [&]() -> bool { return active == 2; },
            [&]() -> void { active = 2; number.value = &con->byte1; activeColor = typeColor[con->type];});
        UIButtonDimmable defSetting("Default", con->color, [&]() -> bool { return active == 3;},
            [&]() -> void { active = 3; number.value = &con->def; activeColor = con->color;});
        UIButtonDimmable minSetting("Min", COLOR_AZURE, [&]() -> bool { return active == 4;},
            [&]() -> void { active = 4; number.value = &con->min; activeColor = COLOR_AZURE;});
        UIButtonDimmable maxSetting("Max", COLOR_RED, [&]() -> bool { return active == 5;},
            [&]() -> void { active = 5; number.value = &con->max; activeColor = COLOR_RED;});

        // UIButtonDimmable sendCC("Send CC", COLOR_YELLOW, [&]() -> bool { return con->type == SEND_CC; },
        //     [&]() -> void { active = 2; number.value = &con->byte1, activeColor = COLOR_YELLOW; con->type = SEND_CC; 
        //                     byte1Setting.color = COLOR_YELLOW; byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});
        // UIButtonDimmable sendPC("Send PC", COLOR_BLUE, [&]() -> bool { return con->type == SEND_PC; },
        //     [&]() -> void { active = 2; number.value = &con->byte1, activeColor = COLOR_BLUE; con->type = SEND_PC; 
        //                     byte1Setting.color = COLOR_BLUE; byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});
        // UIButtonDimmable sendNote("Send Note", COLOR_PINK, [&]() -> bool { return con->type == SEND_NOTE; },
        //     [&]() -> void { active = 2; number.value = &con->byte1, activeColor = COLOR_PINK; con->type = SEND_NOTE; 
        //                     byte1Setting.color = COLOR_PINK; byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});

        UIButtonWithColorFunc plus_10("+10",[&]() -> Color { return (active != 0) ? COLOR_WHITE : COLOR_BLANK;}, 
            [&]() -> void { 
                switch(active) {
                    case 1: *number.value = ((int16_t)*number.value + 10 < 16) ? *number.value + 10 : 127; break;  // channel
                    case 2: *number.value = ((int16_t)*number.value + 10 < 127) ? *number.value + 10 : 127; break; // cc / pc / note
                    case 3: *number.value = ((int16_t)*number.value + 10 < con->max) ? *number.value + 10 : con->max; break; // default
                    case 4: *number.value = ((int16_t)*number.value + 10 < con->max) ? *number.value + 10 : con->max; if(con->def < *number.value) con->def = *number.value; break; //min
                    case 5: *number.value = ((int16_t)*number.value + 10 < 127) ? *number.value + 10 : 127; //max
                }
            });
        UIButtonWithColorFunc plus_1("+1",[&]() -> Color { return (active != 0) ? COLOR_WHITE : COLOR_BLANK;}, 
            [&]() -> void {
                switch(active) {
                    case 1: *number.value = ((int16_t)*number.value + 1 < 16) ? *number.value + 1 : 127;break;  // channel
                    case 2: *number.value = ((int16_t)*number.value + 1 < 127) ? *number.value + 1 : 127; break; // cc / pc / note
                    case 3: *number.value = ((int16_t)*number.value + 1 < con->max) ? *number.value + 1 : con->max; break; // default
                    case 4: *number.value = ((int16_t)*number.value + 1 < con->max) ? *number.value + 1 : con->max; if(con->def < *number.value) con->def = *number.value; break; //min
                    case 5: *number.value = ((int16_t)*number.value + 1 < 127) ? *number.value + 1 : 127; break; //max
                }
            });
        UIButtonWithColorFunc minus_1("-1",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : COLOR_BLANK;}, 
            [&]() -> void {
                switch(active) {
                    case 1: *number.value = ((int16_t)*number.value - 1 > 0) ? *number.value - 1 : 0; break; // channel
                    case 2: *number.value = ((int16_t)*number.value - 1 > 0) ? *number.value - 1 : 0; break; // cc / pc / note
                    case 3: *number.value = ((int16_t)*number.value - 1 > con->min) ? *number.value - 1 : con->min; break; // default
                    case 4: *number.value = ((int16_t)*number.value - 1 > 0) ? *number.value - 1 : 0; break; //min
                    case 5: *number.value = ((int16_t)*number.value - 1 > con->min) ? *number.value - 1 : con->min; if(con->def > *number.value) con->def = *number.value; break; //max
                }
            });
        UIButtonWithColorFunc minus_10("-10",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : COLOR_BLANK;}, 
            [&]() -> void {
                switch(active) {
                    case 1: /* *number.value = ((int16_t)*number.value - 10 > 0) ? *number.value - 10 : 0; */break; // channel
                    case 2: *number.value = ((int16_t)*number.value - 10 > 0) ? *number.value - 10 : 0; break; // cc / pc / note
                    case 3: *number.value = ((int16_t)*number.value - 10 > con->min) ? *number.value - 10 : con->min; break; // default
                    case 4: *number.value = ((int16_t)*number.value - 10 > 0) ? *number.value - 10 : 0; break; //min
                    case 5: *number.value = ((int16_t)*number.value - 10 > con->min) ? *number.value - 10 : con->min; if(con->def > *number.value) con->def = *number.value; break; //max
                }
             });

        UIColorSelector colors(&con->color, [&]() -> void {defSetting.color = con->color; if (active == 3) activeColor = con->color;});

        knobSetting.AddUIComponent(number, Point(4, 0));

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
    }

    void Button_Setting(MidiButtonConfig* con){
        MatrixOS::KEYPAD::Clear();
        MatrixOS::LED::Fill(0);

        string byte1Name[4] = {"SYSTEM", "CC", "Bank", "Note"};
        string byte2Name[4] = {"Value", "Value", "PC", "Velocity"};
        Color typeColor[4] = {COLOR_BLANK, COLOR_YELLOW, COLOR_BLUE, COLOR_PINK};

        int8_t active = 2;
        int8_t* activeVal = &(con->byte1);
        Color activeColor = typeColor[con->type];
        string name = "Button Setting";
        Color chBtnColor = COLOR_LIME;

        UI buttonSetting(name, con->color);

        UI4pxNumberWithColorFunc number([&]() -> Color { return activeColor; }, 3, activeVal);

        UIButtonDimmable globalCH("Use Global Channel", COLOR_WHITE, [&]() -> bool { return con->globalChannel == true; },
            [&]() -> void { con->globalChannel = !con->globalChannel; if (active == 1) activeColor = chBtnColor.ToLowBrightness(!con->globalChannel);});
        UIButtonDimmable chSetting("Channel", chBtnColor, [&]() -> bool { return active == 1; },
            [&]() -> void { active = 1; number.value = &con->channel; activeColor = chBtnColor.ToLowBrightness(!con->globalChannel); number.add1 = true;});
        UIButtonDimmable byte1Setting(byte1Name[con->type], typeColor[con->type], [&]() -> bool { return active == 2; },
            [&]() -> void { active = 2; number.value = &con->byte1; activeColor = typeColor[con->type]; number.add1 = false;});
        UIButtonDimmable byte2Setting(byte2Name[con->type], con->color, [&]() -> bool { return active == 3;},
            [&]() -> void { active = 3; number.value = &con->byte2; activeColor = con->color; number.add1 = false;});

        UIButtonDimmable sendCC("Send CC", COLOR_YELLOW, [&]() -> bool { return con->type == SEND_CC; },
            [&]() -> void { active = 2; number.value = &con->byte1, activeColor = COLOR_YELLOW; con->type = SEND_CC; 
                            byte1Setting.color = COLOR_YELLOW; byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});
        UIButtonDimmable sendPC("Send PC", COLOR_BLUE, [&]() -> bool { return con->type == SEND_PC; },
            [&]() -> void { active = 2; number.value = &con->byte1, activeColor = COLOR_BLUE; con->type = SEND_PC; 
                            byte1Setting.color = COLOR_BLUE; byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});
        UIButtonDimmable sendNote("Send Note", COLOR_PINK, [&]() -> bool { return con->type == SEND_NOTE; },
            [&]() -> void { active = 2; number.value = &con->byte1, activeColor = COLOR_PINK; con->type = SEND_NOTE; 
                            byte1Setting.color = COLOR_PINK; byte1Setting.name = byte1Name[con->type]; byte2Setting.name = byte2Name[con->type];});

        UIButtonWithColorFunc plus_10("+10",[&]() -> Color { return (active != 0) ? COLOR_WHITE : COLOR_BLANK;}, 
            [&]() -> void { *number.value = ((int16_t)*number.value + 10 < ((active == 1) ? 16 : 127)) ? *number.value + 10 : (active == 1) ? 16 : 127;});
        UIButtonWithColorFunc plus_1("+1",[&]() -> Color { return (active != 0) ? COLOR_WHITE : COLOR_BLANK;}, 
            [&]() -> void { *number.value += (*number.value < ((active == 1) ? 16 : 127)); });
        UIButtonWithColorFunc minus_1("-1",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : COLOR_BLANK;}, 
            [&]() -> void { *number.value -= (*number.value > 0); });
        UIButtonWithColorFunc minus_10("-10",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : COLOR_BLANK;}, 
            [&]() -> void { *number.value = ((int16_t)*number.value - 10 > 0) ? *number.value - 10 : 0; });

        UIColorSelector colors(&con->color, [&]() -> void {byte2Setting.color = con->color; if (active == 3) activeColor = con->color;});


        buttonSetting.AddUIComponent(number, Point(4, 0));

        buttonSetting.AddUIComponent(globalCH, Point(4, 4));
        buttonSetting.AddUIComponent(chSetting, Point(5, 4));
        buttonSetting.AddUIComponent(byte1Setting, Point(6, 4));
        buttonSetting.AddUIComponent(byte2Setting, Point(7, 4));

        buttonSetting.AddUIComponent(sendCC, Point(0, 4));
        buttonSetting.AddUIComponent(sendPC, Point(1, 4));
        buttonSetting.AddUIComponent(sendNote, Point(2, 4));

        buttonSetting.AddUIComponent(plus_10, Point(15, 0));
        buttonSetting.AddUIComponent(plus_1, Point(15, 1));
        buttonSetting.AddUIComponent(minus_1, Point(15, 2));
        buttonSetting.AddUIComponent(minus_10, Point(15, 3));

        buttonSetting.AddUIComponent(colors, Point(0, 0));

        buttonSetting.Start();
    }
    
    void Pad_Setting(NotePadConfig* con){
        MatrixOS::KEYPAD::Clear();
        MatrixOS::LED::Fill(0);
        
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


        UI padSetting("Pad Setting", con->color);
        PianoPad rootSelector(Dimension(7, 2), con, true, 0);
        UIItemSelector scaleSelector(Dimension(8, 4), COLOR_ORANGE, &con->scale, 32, scales, scale_names);
        UISelector overlapSelector(Dimension(8, 1), "Overlap", COLOR_YELLOW, 8, &con->overlap);
        UIButtonDimmable alignRootToggle("Aligh Root Key", COLOR_YELLOW, [&]() -> bool { return con->alignRoot; },
            [&]() -> void { con->alignRoot = !con->alignRoot; });
        UIButtonDimmable enfourceScaleToggle("Enfource Scale", COLOR_ORANGE, [&]() -> bool { return con->enfourceScale; },
            [&]() -> void { con->enfourceScale = !con->enfourceScale; });

        padSetting.AddUIComponent(rootSelector, Point(0, 0));
        padSetting.AddUIComponent(scaleSelector, Point(8, 0));
        
        switch(con->type){
            case NOTE_PAD:
                padSetting.AddUIComponent(overlapSelector, Point(0, 3));
                padSetting.AddUIComponent(alignRootToggle, Point(6, 4));
                padSetting.AddUIComponent(enfourceScaleToggle, Point(7, 4));
                break;
            case PIANO_PAD: break;
            case DRUM_PAD: break;
        }

        padSetting.Start();
    }
}