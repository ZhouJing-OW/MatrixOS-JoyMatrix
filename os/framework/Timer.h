#pragma once

#include <stdint.h>
#include "driver/gptimer.h"

// Avoid recuesive include
namespace MatrixOS::SYS
{
  uint32_t Millis(void);
}

class Timer {
 public:
  Timer();
  bool Tick(uint32_t ms, bool continuous_mode = false);
  bool IsLonger(uint32_t ms);
  uint32_t SinceLastTick();
  void RecordCurrent();
  void Reset();
  uint32_t GetTime();

 private:
  uint32_t previous = 0;
};

class MicroTimer {
public:
  MicroTimer(bool start = true);
  uint64_t Micros();
  bool Tick(uint64_t us);
  bool IsLonger(uint64_t us);
  uint64_t SinceLastTick();
  void RecordCurrent();
  void Start();
  void Stop();
  void Reset();
  uint64_t GetTime();

 private:
  gptimer_handle_t gptimer = NULL;
  uint64_t count;
  uint64_t previous = 0;
};

