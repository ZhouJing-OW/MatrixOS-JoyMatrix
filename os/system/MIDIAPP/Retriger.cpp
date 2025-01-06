#include "MidiCenter.h"
  
  namespace MatrixOS::MidiCenter
{
  RetrigInfo retrigInfo;

  const float rateToRatio[16] = {1.0/16, 1.0/12, 1.0/8, 1.0/6, 1.0/4, 1.0/3, 1.0/2, 1, 2, 3, 4, 6, 8, 12, 16, 32};

  void Retrig(Point xy, int8_t rate, int8_t channel, int8_t note, int8_t velocity)
  {
    if (retrigInfo.keyID != 0xFFFF && retrigInfo.triged == true)
    {
      MidiRouter(NODE_KEYPAD, SEND_NOTE, retrigInfo.channel, retrigInfo.note, 0);
    }

    retrigInfo.keyID = Device::KeyPad::XY2ID(xy);
    retrigInfo.rate = rate;
    retrigInfo.channel = channel;
    retrigInfo.note = note;
    retrigInfo.velocity = velocity;

    if (transportState.play)
    {
      uint16_t ticknum = 96 * rateToRatio[rate];
      uint16_t nextStep = (quarterTick / 24) * 24 + 24;
      if (ticknum < 24)
      {
        retrigInfo.quarterTick = quarterTick / 96 * 96 + quarterTick % 96 / ticknum * ticknum + ticknum;
        if (retrigInfo.quarterTick > nextStep) retrigInfo.quarterTick = nextStep;
      }
      else
      {
        if (quarterTick + ticknum > nextStep + 8)
          retrigInfo.quarterTick = nextStep;
        else
          retrigInfo.quarterTick = quarterTick / 24 * 24;
      }
      return;
    }
    retrigInfo.timestamp = MatrixOS::SYS::Millis();
  }

  bool Scan_Retrig()
  {
    if(retrigInfo.keyID == 0xFFFF) return false;
    KeyInfo* keyInfo = Device::KeyPad::GetKey(retrigInfo.keyID);
    if(keyInfo->active() == false)
    {
      if(retrigInfo.triged == true) { MidiRouter(NODE_KEYPAD, SEND_NOTE, retrigInfo.channel, retrigInfo.note, 0);}
      retrigInfo = RetrigInfo();
      return false;
    }

    if(transportState.play)
    {
      if(quarterTick < retrigInfo.quarterTick) return false;
      MidiRouter(NODE_KEYPAD, SEND_NOTE, retrigInfo.channel, retrigInfo.note, retrigInfo.triged ? 0 : retrigInfo.velocity);
      retrigInfo.triged = !retrigInfo.triged;
      uint16_t ticknumHalf = 96 * rateToRatio[retrigInfo.rate] / 2;
      retrigInfo.quarterTick = quarterTick + ticknumHalf - (quarterTick - retrigInfo.quarterTick);
      return true;
    }

    if(MatrixOS::SYS::Millis() < retrigInfo.timestamp + retrigInfo.timedelta) return false;
    MidiRouter(NODE_KEYPAD, SEND_NOTE, retrigInfo.channel, retrigInfo.note, retrigInfo.triged ? 0 : retrigInfo.velocity);
    retrigInfo.triged = !retrigInfo.triged;
    float  interval = rateToRatio[retrigInfo.rate] * tickInterval * 24 / 2;
    retrigInfo.timestamp += interval;
    retrigInfo.timedelta += interval - static_cast<uint32_t>(interval);
    return true;
  }
}