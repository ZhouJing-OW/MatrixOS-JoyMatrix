#pragma once

struct SaveVarInfo {
  void** ptr;
  size_t size;
  uint16_t count;
  uint32_t tellp = 0; // The position of the file pointer,written by FATFS::ListLoad
};

struct TransportState {
  bool play = false;
  bool pause = false;
  bool record = false;
  bool mute = false;
  bool solo = false;
  bool loop = false;
  bool metronome = false;
  bool autoGrouth = false;
  bool undo = false;
  bool channelMute[16] = {};
  bool channelSolo[16] = {};
};

struct AnalogConfig {
  char name[4];
  uint16_t max;
  uint16_t min;
  uint16_t middle;
  uint16_t deadZone = 100;
};

struct TabConfig{
  char name[12];
  Color color = COLOR_BLANK;
  uint8_t subTab = 0;
  uint8_t subMax = 3;
};

struct ChannelConfig {
  int8_t selectCC = 3;
  int8_t muteCC = 118;
  int8_t soloCC = 119;
  int8_t padType[16]; // PianoPad \ NotePad \ DrumPad
  uint8_t activePadConfig[16][3];
  uint8_t activeDrumNote[16];
  int8_t bankMSB[16];
  int8_t bankLSB[16];
  int8_t PC[16];
  Color color[16];
};

struct NotePadConfig {
  bool enfourceScale = true;
  bool alignRoot = true; // Only works when overlap is set to 0
  bool globalChannel = true;
  int8_t shift = 0;
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
  bool toggleMode = false;
  int8_t type = SEND_CC; // 0：Sys, 1：CC, 2：PC, 3：Note
  int8_t channel = 0 ;   // channel
  int8_t byte1 = 0;    // CC \ PC bank \ Note number
  int8_t byte2 = 127;  // CC value \ PC \ velocity
  Color color = COLOR_CYAN;
};

struct KnobConfig {
  uint16_t pos = 0xFFFF; // Changed by the knob center.
  bool lock = false;
  int8_t type = SEND_NONE;// 0：none, 1：CC, 2：PC,
  int8_t channel = 0;
  int8_t byte1 = 0; // CC \ PC bank
  int16_t byte2 = 0; // CC value \ PC
  int16_t min = 0;
  int16_t max = 127;
  int16_t def = 0;
  Color color = COLOR_RED;

  void SyncTo(int16_t* dest) { *dest = byte2;}
  void Get(int16_t* src) { byte2 = *src;}
};


// struct PCBankConfig {
//   int8_t bank[16];
//   int8_t pc[16];
// };

// size_t ConfigSize[7] = {
//   sizeof(TransportState), 
//   sizeof(AnalogConfig), 
//   sizeof(TabConfig), 
//   sizeof(ChannelConfig), 
//   sizeof(NotePadConfig), 
//   sizeof(MidiButtonConfig), 
//   sizeof(KnobConfig) };