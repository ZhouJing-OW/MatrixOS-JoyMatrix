#include "MidiCenter.h"

#define STEP_SUBDIVISION 12
namespace MatrixOS::midiCenter
{
  struct ClipInfo
  {
    uint8_t clipID;       // 16 * 8;
    uint8_t barNum;       // max = 8;
    uint8_t barStep;      // max = 16;
    uint8_t speed;        // 4x, 3x, 2x, 1x, 1/2x, 1/3x, 1/4x, 1/8x
    uint8_t noteLength;   // 5% - 100%
    uint8_t quantize;
    int16_t barID[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
  };

  struct SEQ_BarNotes
  {
    uint16_t barID;
    uint8_t notePos[64];
    SEQ_Note note[64];
    SEQ_Component component[16];
  };

  struct SEQ_BarParams
  {
    uint16_t barID;
    SEQ_Parameter parameter[8][16];
  };

  struct SEQ_Note
  {
    int8_t note     = -1;
    int8_t velocity = 0;
    int8_t gate     = 0;
  };

  struct SEQ_Component
  {
    int8_t retrig   = 0;
    int8_t cycle    = 0b00000000;
    int8_t chance   = 100;
    int8_t random   = 0;
  };

  struct SEQ_Parameter
  {
    int8_t number   = -1;  // number 0 - 31 :knobs, number 32 : modWheel, number 33-34 : pitchWheel
    int8_t value    = -1;
  };

  class Sequencer
  {
    public:
    ClipInfo* clipInfo;
    SEQ_BarNotes*  barNotes;
    SEQ_BarParams* barParams;
    std::multimap<uint16_t, SEQ_Note*> notes;        // position, note

    uint32_t timer;
    uint16_t playHead;
    uint16_t PosInterval;

    Sequencer() { };
    void Scan();

    private:
    void Trigger();
    void Jump();
    void Chance();
    void Random();
    void Retrig();

  };

  class sizeofSequencer
  {
    size_t size1 = sizeof(SEQ_BarNotes);
    size_t size2 = sizeof(SEQ_BarParams);
    size_t size3 = sizeof(Sequencer);
    size_t size4 = sizeof(ClipInfo);
  };
  

}