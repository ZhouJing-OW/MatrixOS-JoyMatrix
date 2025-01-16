#pragma once
#include "MidiCenter.h"
#include "SeqData.h"

namespace MatrixOS::MidiCenter
{
  class Sequencer : public Node
  {
    const uint16_t speedToQuarterTick[10] = {6, 8, 12, 16, 24, 32, 48, 96, 192, 384};
    const float   retrigDecayRatio[4] = {1, 0.8, 0.65, 0.5};
    const uint8_t flamVelocity[4] = {16, 32, 64, 96};

   public:
    uint32_t stepTime = 0;
    int16_t playHead = 0;
    int16_t buffHead = 0;
    int16_t capHead = 0;
    uint8_t clipNum = 0;
    uint8_t quarterTickPerStep;
    uint16_t scale = 0x0FFF;
    std::queue<std::pair<uint32_t, SEQ_Note>> notesQueue; // OnTime, Note
    std::map<uint8_t, std::pair<uint32_t, SEQ_Step*>> recNotes; // note, head, step
    std::map<const SEQ_Note*, uint8_t> cycleStep;
    double stepInterval = 0;
    bool firstStepBuff = false;
    bool noteBuff = false;
    bool end = true;
    bool recording = false;
    bool capturing = false;

    Sequencer(uint8_t channel)
    {
      thisNode = NODE_SEQ;
      this->channel = channel;
    }

    void Scan();
    void Record(uint8_t channel, uint8_t byte1, uint8_t byte2);
    void Capture(uint8_t channel, uint8_t byte1, uint8_t byte2);
    void End();

   private:
    bool HasNoteInRange(uint8_t channel, uint8_t clipNum, int8_t startBar, int8_t endBar);
    void MoveHead(int16_t& head);
    void GetNoteQueue(bool firstStep = false);
    void GetAutomQueue();
    void FirstStepBuff();
    void Trigger();
    void SetRecNoteGate(uint8_t note = 255);
    
    bool Chance     (const SEQ_Step* step);
    bool Chance     (const SEQ_Note* note);
    bool Cycle      (const SEQ_Step* step);
    bool Cycle      (const SEQ_Note* note);
    void Retrig     (const SEQ_Step* step, uint8_t* retrig, uint8_t* retrigDecay);
    void Retrig     (const SEQ_Note* note, uint8_t* retrig, uint8_t* retrigDecay);
    void Flam       (const SEQ_Step* step, uint8_t* flamTime, uint8_t* flamVel);
    void Flam       (const SEQ_Note* note, uint8_t* flamTime, uint8_t* flamVel);
    void PitchShift (const SEQ_Step* step, int8_t* octaveShift, int8_t* pitchShift);
    void PitchShift (SEQ_Note* note, int8_t octaveShift, int8_t pitchShift);
  };
  
  class sizeofSequencer
  {
    size_t size(){
      size_t size;
      size = sizeof(std::bitset<256>);
      size = sizeof(SEQ_Note);
      size = sizeof(SEQ_Param);
      size = sizeof(SEQ_Autom);
      size = sizeof(SEQ_Step);
      size = sizeof(SEQ_Pattern);
      size = sizeof(SEQ_Song);
      size = sizeof(Sequencer);
      size = sizeof(SEQ_Clip);
      size = sizeof(SEQ_DataStore);
      return size;
    }
  };

}