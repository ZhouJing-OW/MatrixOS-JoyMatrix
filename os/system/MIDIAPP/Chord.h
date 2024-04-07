#pragma once
#include "MatrixOS.h"



struct ChordConfig // Only the base type is allowed within the structure
{
  bool autoVoicing        = false;
  bool randomTreble      = false;
  bool randomDrop         = false;
  bool seventh            = true;
  uint8_t maxVoices       = 4;
  uint8_t bassRange       = 2;
  uint8_t trebleRange     = 2;
  uint8_t harmony         = 100;

  bool operator==(const ChordConfig& other) const { return memcmp(this, &other, sizeof(ChordConfig)) == 0; }
  bool operator!=(const ChordConfig& other) const { return !(*this == other); }
  ChordConfig& operator=(const ChordConfig& other) { memcpy(this, &other, sizeof(ChordConfig)); return *this; }
};

namespace MatrixOS::MidiCenter
{
  extern ChannelConfig* channelConfig;
  extern NotePadConfig* notePadConfig;

  enum ChordType : uint8_t { 
    TRI_Major,            TRI_Minor,            TRI_Diminished,       TRI_Augmented,
    SEVEN_Major,          SEVEN_Minor,          SEVEN_Dominant,       SEVEN_HalfDiminished,
    SEVEN_Diminished,     SEVEN_MinorMajor,     SEVEN_AugmentedMajor, SEVEN_Augmented,
  };

  const uint16_t chordType[12] = { // left to right
    0b000010010001, 0b000010001001, 0b000001001001, 0b000100010001, 
    0b100010010001, 0b010010001001, 0b010010010001, 0b010001001001, 
    0b001001001001, 0b100010001001, 0b100100010001, 0b010100010001, 
  };

  class Chord : public Node
  {
    public:
    ChordConfig* config;
    ChordConfig* activeConfig;
    uint16_t chordCheck[12];
    uint16_t scale = 0x0FFF;
    uint16_t scalePrv = 0xFFFF;
    uint16_t keyScale;
    uint32_t createTime = 0;
    uint32_t releaseTime = 0;
    uint8_t lastRoot, lastTemp = 255, lastBass, lastTreble;
    uint8_t drop = 0;
    bool sus2, sus4, inputEmpty;
    uint8_t activeLabel = 0;
    std::map<uint8_t, NoteInfo> tempList;
    int8_t currentVoice[4] = {-1, -1, -1, -1};

    Chord(uint8_t channel, ChordConfig* config, uint8_t configNum)
    {
      if (channelConfig == nullptr || notePadConfig == nullptr)
        MLOGE("MidiNode Chord", "No channel config or notepad config found. scale set to CHOMATIC.");
      thisNode = NODE_CHORD;
      this->config = config;
      this->channel = channel;
      SetActiveConfig(configNum);
    }
    ~Chord()
    {
      for(auto it : tempList)
        MidiRouter(NODE_CHORD, SEND_NOTE, channel, it.first, 0);
    }

    virtual void Scan();

    private:
    virtual void OutListNoteOff(uint8_t note);
    void CreateChord();
    void CheckInput(uint8_t* voice);
    void CheckVoices(uint8_t* voice);
    uint32_t Voicing(uint8_t* voice);
    void SendChord(std::vector<uint8_t>& chord, uint8_t* voice);

    void SetActiveConfig(uint8_t num);
    void GetCurrentScale();
    void MoveToTemp(uint8_t note);
    };
}