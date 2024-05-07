#pragma once
#include "MidiCenter.h"
#include "SeqData.h"

namespace MatrixOS::MidiCenter
{
  extern SEQ_DataStore*     seqData;

  class Sequencer : public Node
  {
  public:
    int16_t playHead = 0;
    int16_t buffHead = 0;
    uint8_t clipNum = 0;
    uint8_t speedTick;
    std::queue<std::pair<uint32_t, SEQ_Note>> notesQueue; // OnTime, Note
    double interval = 0;
    bool firstStepBuff = false;
    bool noteBuff = false;
    bool end = true;

    void Scan();

    Sequencer(uint8_t channel)
    {
      thisNode = NODE_SEQ;
      this->channel = channel;
    }
    void FirstStepBuff();

    const uint8_t speedToHalfTick[10] = {3, 4, 6, 8, 12, 12, 16, 24, 48, 96};

  private:
    void End();
    void GetNoteQueue(bool firstStep = false);
    void MoveHead(int16_t& head);
    void GetAutomQueue();
    void Trigger();
    void Jump();
    void Chance();
    void Random();
    void Retrig();
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