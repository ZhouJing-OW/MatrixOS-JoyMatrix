#pragma once
#include "MatrixOS.h"
#include <map>


enum ArpType : uint8_t { // 12 types
  ARP_Up,
  ARP_Down,
  ARP_Converge,
  ARP_Diverge,
  ARP_PinkUp,
  ARP_PinkDown,
  ARP_ThumbUp,
  ARP_ThumbDown,
  ARP_Random,
  ARP_RandomOther,
  ARP_RandomOnce,
  ARP_ByOrder,
};

struct ArpConfig
{
  bool syncBeat           = true;
  bool skip               = false;
  bool forBackward        = false;
  bool repeatEnds         = false;
  uint8_t pattern         = 1;          // 0b00000001 - 0b11111111
  int16_t rate            = 4;          // 1/16, 1/12, 1/8, 1/6, 1/4, 1/3, 1/2, 1, 2, 3, 4, 6, 8, 12, 16, 32
  int16_t patternLength   = 1;          // 1 - 8
  int16_t chance          = 100;        // 0 - 100
  int16_t gate            = 6;          // 5% , 10%, 15%, 20%, 30%, 40%, def 50%, 60%, 70%, 80%, 90%, 100%, 150%, 200%, 300%, 400%
  int16_t type            = ARP_Up;     // 0 - 11
  int16_t octaveRange     = 1;          // 1 - 6 
  int16_t noteRepeat      = 1;          // 1 - 8
  int16_t velDecay        = 0;          // 0 - 48
  int16_t velocity[8]     = {127, 127, 127, 127, 127, 127, 127, 127};
};

namespace MatrixOS::MidiCenter
{
  class Arpeggiator
  {
    public:
    ArpConfig* config;
    uint8_t channel;
    uint8_t inputVelocity;
    std::map<uint8_t,uint8_t> inputList; // note , order
    std::map<uint8_t,uint8_t> inputListPrv; // note, order
    std::vector<uint8_t> arpList;
    std::vector<uint8_t> arpArrange;
    std::vector<int16_t*> configPtr;
    uint16_t configPrv[8];
    bool syncBeatPrev;
    bool skipPrev;
    bool forBackwardPrev;
    bool repeatEndsPrev;

    const float rateToRatio[16] = {1.0/16, 1.0/12, 1.0/8, 1.0/6, 1.0/4, 1.0/3, 1.0/2, 1, 2, 3, 4, 6, 8, 12, 16, 32};
    const float gateToRatio[16] = {0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 1.5, 2, 3, 4};

    Arpeggiator(ArpConfig* config, uint8_t channel) { 
      this->config = config;
      this->channel = channel;
      activeLabel = 4;
      arpList.reserve(8);
      arpArrange.reserve(32);
      configPtr = {&config->rate,         &config->patternLength,   &config->chance,        &config->gate,
                   &config->type,         &config->octaveRange,     &config->noteRepeat,    &config->velDecay};
    }

    uint32_t arpTimer;
    double arpInterval;
    double intervalDelta;
    uint32_t gateLength;
    uint8_t decayNow;
    uint8_t activeLabel;

    void Scan();

    int8_t currentStep = -1;
    uint8_t currentOctave = 0;
    uint8_t currentRepeat = 0;

    private:
    uint8_t currentArpNote = 0;

    bool synced;

    void CheckVarChange();
    void Trigger();
    void ArpEnd(bool reset = true);
    void ArpStart(bool reset = true);
    bool SyncBeat();
    bool GetStep();
    void GenerateNoteList();
    void NoteArrange();
    void OctaveExpansion();
    void forBackward();
  };

  extern std::map<uint8_t, Arpeggiator*> arpeggiators; // channel, Arpeggiator
  
  // size_t sizeofArpConfig = sizeof(ArpConfig);
  // size_t sizeofArp = sizeof(Arpeggiator);
  
}
