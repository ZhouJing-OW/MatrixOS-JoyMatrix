#pragma once
#include "system/MIDIHold.h"

struct TransportState {
    bool play = false;
    bool record = false;
    bool mute = false;
    bool solo = false;
    bool loop = false;
    bool autoGrouth = false;
    bool metronome = false;
    bool undo = false;
    bool channelMute[16];
    bool channelSolo[16];
};

struct AnalogConfig {
    string name;
    uint16_t max;
    uint16_t min;
    uint16_t middle;
    uint16_t deadZone = 50;
};

struct TabConfig{
    string name = "";
    Color color = COLOR_BLANK;
    uint8_t subTab = 0;
    uint8_t subMax = 3;
};

struct ChannelConfig {
    int8_t selectCC = 120;
    int8_t muteCC = 119;
    int8_t soloCC = 118;
    int8_t type[16];
    Color color[16];
};

enum PadType : int8_t {
    NOTE_PAD = 0,
    PIANO_PAD = 1,
    DRUM_PAD = 2,
};

struct NotePadConfig {
    Color color = COLOR_PURPLE;
    Color rootColor = COLOR_PINK;
    bool enfourceScale = true;
    bool alignRoot = true; // Only works when overlap is set to 0
    bool globalChannel = true;
    int8_t type = NOTE_PAD;
    int8_t channel = 0;
    int8_t rootKey = 0;
    int8_t octave = 3;
    int8_t overlap = 4;
    uint16_t scale = CHROMATIC;
};

struct MidiButtonConfig {
    bool active = false;  
    bool globalChannel = false;
    int8_t type = SEND_CC; // 0：Sys, 1：CC, 2：PC, 3：Note
    int8_t channel = 0 ;   // channel
    int8_t byte1 = 0;    // CC \ PC bank \ Note number
    int8_t byte2 = 127;  // CC value \ PC \ velocity
    Color color = COLOR_CYAN;
};

struct KnobConfig {
    bool enable = false;
    bool changed = false;
    bool lock = false;
    int8_t type = SEND_CC;// 0：Sys, 1：CC, 2：PC,
    int8_t channel = 0;
    int8_t byte1 = 0; // CC \ PC bank
    int8_t byte2 = 0; // CC value \ PC
    int8_t min = 0;
    int8_t max = 127;
    int8_t def = 0;
    Color color = COLOR_RED;
}; 