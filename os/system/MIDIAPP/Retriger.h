#include "MatrixOS.h"
#include "MidiCenter.h"

  namespace MatrixOS::MidiCenter
{
  struct RetrigInfo {
    uint16_t keyID = 0xFFFF;
    int8_t rate = 0;
    int8_t channel = 0;
    int8_t note = 0;
    int8_t velocity = 0;
    uint32_t timestamp = 0;
    uint32_t quarterTick = 0;
    float timedelta = 0;
    bool triged = false;
  };

  void Retrig(Point xy, int8_t rate, int8_t channel, int8_t note, int8_t velocity);
  bool Scan_Retrig();
}