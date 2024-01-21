#include "MatrixOS.h"
#include "ui\UI.h"

namespace MatrixOS::Component {

    void Tab_ToggleSub(TabConfig* con) {
        if (con->subTab < con->subMax - 1){
            con->subTab++;
        } else con->subTab = 0;
    }

    void Knob_Function(KnobConfig* con, int16_t* value) {
        if (con->enable) {
            switch (con->type) {
                case SEND_NONE: break;
                case SEND_CC: MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, con->channel, con->byte1, *value)); break;
                case SEND_PC: MatrixOS::MIDI::Send(MidiPacket(0, ProgramChange, con->channel, con->byte1, *value)); break;
            }
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

        UI4pxNumberWithColorFunc CCVal([&]() -> Color { return (active != 0) ? con->color[n] : activeColor; }, 3, &channel);

        UIButtonDimmable channelBtn("CC for Select", con->color[n], [&]() -> bool { return active == 0; },
            [&]() -> void { active = 0; activeColor = con->color[n]; CCVal.value = &channel;});
        UIButtonDimmable selectCC("CC for Select", COLOR_LIME, [&]() -> bool { return active == 1; },
            [&]() -> void { active = 1; activeColor = COLOR_LIME; CCVal.value = &con->SelectCC;});
        UIButtonDimmable muteCC("CC for Mute", COLOR_RED, [&]() -> bool { return active == 2; },
            [&]() -> void { active = 2; activeColor = COLOR_RED; CCVal.value = &con->MuteCC;});
        UIButtonDimmable soloCC("CC for Solo", COLOR_BLUE, [&]() -> bool { return active == 3; },
            [&]() -> void { active = 3; activeColor = COLOR_BLUE; CCVal.value = &con->SoloCC; });
        UIButtonWithColorFunc add_10("+10",[&]() -> Color { return (active != 0) ? COLOR_WHITE : COLOR_BLANK;}, 
            [&]() -> void { *CCVal.value = ((int16_t)*CCVal.value + 10 < 127) ? *CCVal.value + 10 : 127;});
        UIButtonWithColorFunc add_1("+1",[&]() -> Color { return (active != 0) ? COLOR_WHITE : COLOR_BLANK;}, 
            [&]() -> void { *CCVal.value += (*CCVal.value < 127); });
        UIButtonWithColorFunc dim_1("-1",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : COLOR_BLANK;}, 
            [&]() -> void { *CCVal.value -= (*CCVal.value > 0); });
        UIButtonWithColorFunc dim_10("-10",[&]() -> Color { return (active != 0) ? Color(0xFFFFFF).ToLowBrightness() : COLOR_BLANK;}, 
            [&]() -> void { *CCVal.value = ((int16_t)*CCVal.value - 10 > 0) ? *CCVal.value - 10 : 0; });

        UIColorSelector colors(Dimension(3,4), &con->color[n], 
            [&]() -> void { active = 0; channelBtn.color = con->color[n]; activeColor = con->color[n]; CCVal.value = &channel;});

        channelSetting.AddUIComponent(channelBtn, Point(0, 4));
        channelSetting.AddUIComponent(selectCC, Point(5, 4));
        channelSetting.AddUIComponent(muteCC, Point(6, 4));
        channelSetting.AddUIComponent(soloCC, Point(7, 4));
        channelSetting.AddUIComponent(colors, Point(0, 0));
        channelSetting.AddUIComponent(CCVal, Point(4, 0));
        channelSetting.AddUIComponent(add_10, Point(15, 0));
        channelSetting.AddUIComponent(add_1, Point(15, 1));
        channelSetting.AddUIComponent(dim_1, Point(15, 2));
        channelSetting.AddUIComponent(dim_10, Point(15, 3));

        channelSetting.Start();
    }

    void Knob_Setting(KnobConfig* con) {

    }

    void Button_Setting(MidiButtonConfig* con){

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
        int8_t offset = 0;
        PianoPad rootSelector(Dimension(7, 2), offset, con, true);
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