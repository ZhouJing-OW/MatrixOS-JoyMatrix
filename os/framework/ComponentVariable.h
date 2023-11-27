#pragma once
# include <functional>

struct TabConfig{
    public:
     string name = "";
     Color color = COLOR_BLANK;
     uint8_t subTab = 0;
     uint8_t subMax = 3;

     void ToggleSub(){
        if (subTab < subMax){
            subTab++;
        } else subTab = 0;
     }
};

struct AnalogConfig {
    public:
     string name = "";
     uint16_t max = 4095;
     uint16_t min = 0;
     uint16_t middle = 2048;
     uint16_t deadZone = 50;
};

struct NotePadConfig {
    public:
     string name = "";
     uint16_t scale = CHROMATIC;
     uint8_t rootKey = 0;
     int8_t octave = 3;
     uint8_t channel = 0;
     uint8_t overlap = 4;
     Color color = COLOR_VIOLET;
     Color rootColor = COLOR_FUCHSIA;
     bool enfourceScale = true;
     bool alignRoot = true;  // Only works when overlap is set to 0
     bool globalChannel = true;
};

struct MidiButtonConfig {
    public:
     string name = "";
     Color color = COLOR_CYAN;
     uint8_t type = 0;      // 0: Command Change, 1: Program Change , 2: Note
     uint8_t channel = 0;   // channel
     uint8_t value1 = 0;    // CC\PC\Note number
     uint8_t value2 = 127;  // value
     bool globalChannel = false;
     bool pressed = false; 
};

struct ChannelButtonConfig {
    public:
     string name = "";
     Color color = COLOR_GREEN;
     uint8_t channel = 0;
     uint8_t channelSelectCC = 120;
     uint8_t channelMuteCC = 119;
     uint8_t channelSoloCC = 118;
     bool pressed = false;
};

namespace MatrixOS::MIDI{
    bool Send(MidiPacket midiPacket, uint16_t timeout_ms = 0);
}

struct KnobConfig {
    public:
     string name = "";
     Color color = COLOR_WHITE;
     uint8_t channel = 0;  // channel
     uint8_t value1 = 0;   // CC/PC/Note number
     int32_t value2 = 0;   // value
     int32_t min = 0;
     int32_t max = 127;
     int32_t def = 0;
     bool enable = true;
     std::function<void()> callback = nullptr;
     std::function<void()> shiftCallback = callback;

     void SetSendCC() {
        callback = [&]() -> void {
            MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, channel, value1, value2));
        };
     };

     void SetSendPC() { 
        callback = [&]() -> void {
            MatrixOS::MIDI::Send(MidiPacket(0, ProgramChange, channel, value1, value2)); 
        };
     };
};