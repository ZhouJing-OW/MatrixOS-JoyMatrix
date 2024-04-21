#pragma once
#include "MidiCenter.h"
#include "SeqData.h"

namespace MatrixOS::MidiCenter
{
  class Sequencer
  {
  public:
    uint8_t channel;
    SEQ_DataStore* dataStore;
    uint8_t current = 0;
    uint8_t clipNum[16];
    int16_t playHead[16];
    std::multimap<uint16_t, SEQ_Note>       Buffer_Notes;       // half tick, note
    std::multimap<uint16_t, SEQ_Param>      Buffer_Params;      // half tick, param
    uint32_t halfTick_last;
    uint16_t halfTick_wait;

    void Scan();

  private:
    void GetBuffer( uint8_t index);
    void GetAutom();
    void Trigger();
    void Jump();
    void Chance();
    void Random();
    void Retrig();
  };

  class SongInfo
  {

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
      size = sizeof(Sequencer);
      size = sizeof(SEQ_Clip);
      size = sizeof(SEQ_DataStore);
      return size;
    }
  };

}