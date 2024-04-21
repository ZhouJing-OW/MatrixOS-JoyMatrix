#pragma once

struct SaveVarInfo {
  void** ptr;
  size_t size;
  uint16_t count;
  uint32_t filePos = 0; // The position of the file pointer,written by FATFS::ListLoad
};

struct ProjectConfig
{
  int16_t bpm = 120;
  int16_t swing = 0;
  bool loop = false;
};

struct TransportState {
  bool play = false;
  bool pause = false;
  bool record = false;
  bool mute = false;
  bool solo = false;
  bool undo = false;
  bool metronome = false;
  bool autoGrouth = false;
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
  uint8_t activeNote[16];
  bool channelMute[16];
  bool channelSolo[16];
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
  bool lock       = false;
  bool middleMode = false;
  SendType type = SEND_NONE;// 0：none, 1：CC, 2：PC,
  union DataBank
  {
    struct
    {
      int8_t  channel;
      int8_t  byte1; // CC \ PC bank
      int16_t byte2; // CC value \ PC
    };
    int16_t* varPtr = nullptr;
  }data;

  int16_t min = 0;
  int16_t max = 127;
  int16_t def = 0;
  Color color = COLOR_RED;

  int16_t Value() 
  {
    if (type == SEND_NONE) {  return (data.varPtr != nullptr) ? *data.varPtr : def; }
    else return data.byte2;
  }

  void SetType(SendType type) { if(type != SEND_NONE)this->type = type; }

  void SetPtr(int16_t* ptr) { data.varPtr = ptr; type = SEND_NONE; }

  int16_t* GetPtr() {if(type == SEND_NONE) return data.varPtr; else return &data.byte2;}
  
  void SetValue(int16_t value)
  {
    if(type == SEND_NONE) { if(data.varPtr != nullptr) *data.varPtr = value; }
    else data.byte2 = value;
  }

  KnobConfig& operator=(const KnobConfig& knob)
  {
    this->type = knob.type;
    this->pos = knob.pos;
    this->lock = knob.lock;
    this->middleMode = knob.middleMode;
    if (type == SEND_NONE)
    {
      this->data.varPtr = knob.data.varPtr;
    }
    else
    {
      this->data.channel = knob.data.channel;
      this->data.byte1 = knob.data.byte1;
      this->data.byte2 = knob.data.byte2;
    }
    this->min = knob.min;
    this->max = knob.max;
    this->def = knob.def;
    this->color = knob.color;
    return *this;
  }
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