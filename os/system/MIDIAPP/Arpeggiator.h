#pragma once
#include "MatrixOS.h"
#include "MidiCenter.h"
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

struct ArpConfig // Only the base type is allowed within the structure
{
  bool timeSync           = true;
  bool skip               = false;
  bool forBackward        = false;
  bool repeatEnds         = false;
  uint8_t pattern         = 1;          // 0b00000001 - 0b11111111
  int16_t type            = ARP_Up;     // 0 - 11
  int16_t rate            = 4;          // 1/16, 1/12, 1/8, 1/6, 1/4, 1/3, 1/2, 1, 2, 3, 4, 6, 8, 12, 16, 32
  int16_t octaveRange     = 1;          // 1 - 6 
  int16_t noteRepeat      = 1;          // 1 - 8  
  int16_t patternLength   = 1;          // 1 - 8
  int16_t chance          = 100;        // 0 - 100
  int16_t gate            = 6;          // 5% , 10%, 15%, 20%, 30%, 40%, def 50%, 60%, 70%, 80%, 90%, 100%, 150%, 200%, 300%, 400%
  int16_t velDecay        = 0;          // 0 - 48
  int16_t velocity[8]     = {127, 127, 127, 127, 127, 127, 127, 127};

  bool NeedReset(const ArpConfig& other){
    return (forBackward != other.forBackward) || (repeatEnds != other.repeatEnds) || 
           (type != other.type) || (noteRepeat != other.noteRepeat) || (octaveRange != other.octaveRange);
  }
  
  bool operator==(const ArpConfig& other) const { return memcmp(this, &other, sizeof(ArpConfig)) == 0; }
  bool operator!=(const ArpConfig& other) const { return !(*this == other); }
  ArpConfig& operator=(const ArpConfig& other) { memcpy(this, &other, sizeof(ArpConfig)); return *this; }
};

namespace MatrixOS::MidiCenter
{
  class Arpeggiator : public Node
  {
  public:
    ArpConfig* configRoot;
    ArpConfig* config;
    ArpConfig configPrv;
    uint8_t inputVelocity;
    std::vector<uint8_t> arpList;
    std::vector<uint8_t> arpArrange;
    std::vector<int16_t*> configPtr;

    const float rateToRatio[16] = {1.0/16, 1.0/12, 1.0/8, 1.0/6, 1.0/4, 1.0/3, 1.0/2, 1, 2, 3, 4, 6, 8, 12, 16, 32};
    const float gateToRatio[16] = {0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 1.5, 2, 3, 4};

    Arpeggiator(uint8_t channel, ArpConfig* configRoot, uint8_t configNum) {
      thisNode = NODE_ARP;
      this->channel = channel;
      this->configRoot = configRoot;
      this->configNum = configNum;
      activeLabel = 0;
      arpList.reserve(8);
      arpArrange.reserve(32);
      SetActiveConfig(configNum);
    }

    bool empty = true;
    bool synced = false;
    int8_t currentStep      = -1;
    uint8_t currentOctave   = 0;
    uint8_t currentRepeat   = 0;
    uint8_t currentArpNote  = 0;
    uint32_t arpTimer       = 0;
    double arpInterval      = 0;
    double intervalDelta    = 0;
    uint32_t gateLength     = 0;
    uint8_t decayNow        = 0;
    uint8_t activeLabel     = 0;
    uint8_t lastLabel       = 0;

    virtual void Scan();
    void SetActiveConfig(uint8_t num);

  private:

    void CheckVarChange();
    void Trigger();
    void ArpEnd(bool reset = true);
    void ArpStart(bool reset = true);
    bool StartCheck();
    bool GetStep();
    void GenerateNoteList();
    void NoteArrange();
    void Reverse();
    void forBackward();
  };
  
  // size_t sizeofArpConfig = sizeof(ArpConfig);
  // size_t sizeofArp = sizeof(Arpeggiator);
  
}
