#include <functional>
#include "MatrixOS.h"
#include "AnalogInput.h"

namespace Device::AnalogInput
{
  extern AnalogConfig RX;
  extern AnalogConfig RY;

  int8_t* upDown = nullptr;
  int8_t UD_max;
  int8_t UD_min;
  int8_t UD_step;
  bool UD_loop;
  std::function<void()> UD_callback;
  Timer UD_timer;

  int8_t* leftRight = nullptr;
  int8_t LR_max;
  int8_t LR_min;
  int8_t LR_step;
  bool LR_loop;
  std::function<void()> LR_callback;
  Timer LR_timer;

  const uint16_t timeInterval = 200;
  uint16_t UD_interval;
  uint16_t LR_interval;

  void DirectPad() {
    // if (GetRocker(RY) != 0) MLOGD("Rocker","ry = %d", GetRocker(RY));
    // if (GetRocker(RX) != 0) MLOGD("Rocker","rx = %d", GetRocker(RX));

    if (upDown != nullptr && UD_max > UD_min)
    { 
      if (GetRocker(RY) > 64 && UD_timer.IsLonger(UD_interval))
      {
        UD_interval = timeInterval;
        UD_timer.RecordCurrent();
        *upDown = (*upDown - UD_step >= UD_min) ? *upDown - UD_step : (UD_loop ? UD_max : UD_min);
        if (UD_callback != nullptr)
          UD_callback();
        // MLOGD("DirectPad", "_up: %d", *upDown);
      }
      else if (GetRocker(RY) < -64 && UD_timer.IsLonger(UD_interval))
      {
        UD_interval = timeInterval;
        UD_timer.RecordCurrent();
        *upDown = (*upDown + UD_step <= UD_max) ? *upDown + UD_step : (UD_loop ? UD_min : UD_max);
        if (UD_callback != nullptr)
          UD_callback();
        // MLOGD("DirectPad", "_down: %d", *upDown);
      }
      else if (GetRocker(RY) > -16 && GetRocker(RY) < 16)
        UD_interval = 0;
    }

    if (leftRight != nullptr && LR_max > LR_min)
    {
      if (GetRocker(RX) > 64 && LR_timer.IsLonger(LR_interval))
      { 
        LR_interval = timeInterval;
        LR_timer.RecordCurrent();
        *leftRight = (*leftRight + LR_step <= LR_max) ? *leftRight + LR_step : (LR_loop ? LR_min : LR_max);
        if (LR_callback != nullptr)
          LR_callback();
        // MLOGD("DirectPad", "_left: %d", *leftRight);
      }
      else if (GetRocker(RX) < -64 && LR_timer.IsLonger(LR_interval))
      {
        LR_interval = timeInterval;
        LR_timer.RecordCurrent();
        *leftRight = (*leftRight - LR_step >= LR_min) ? *leftRight - LR_step : (LR_loop ? LR_max : LR_min);
        if (LR_callback != nullptr)
          LR_callback();
        // MLOGD("DirectPad", "_right: %d", *leftRight);
      }
    }
    else if (GetRocker(RX) > -16 && GetRocker(RX) < 16)
      LR_interval = 0;
  }

  void SetUpDown(int8_t* up_down, int8_t max, int8_t min, int8_t step, bool loop, std::function<void()> callback) {
    upDown = up_down;
    UD_max = max;
    UD_min = min;
    UD_step = step;
    UD_loop = loop;
    UD_callback = callback;
  }

  void SetLeftRight(int8_t* left_right, int8_t max, int8_t min, int8_t step,  bool loop, std::function<void()> callback) {
    leftRight = left_right;
    LR_max = max;
    LR_min = min;
    LR_step = step;
    LR_loop = loop;
    LR_callback = callback;
  }

  void DisableDirectPad() { leftRight = nullptr; upDown = nullptr;}
  void DisableLeftRight() { leftRight = nullptr;}
  void DisableUpDown() { upDown = nullptr;}

  int8_t* GetUDPtr() { return upDown; }

  int8_t* GetLRPtr() { return leftRight; }

  bool UDIsLonger(uint32_t ms) { return UD_timer.IsLonger(ms); }

  bool LRIsLonger(uint32_t ms) { return LR_timer.IsLonger(ms); }
}