#pragma once

#include "MatrixOS.h"
#include "MatrixOS.h"
#include "Device.h"
#include "timers.h"
  
namespace Device::AnalogInput
{
  void Scan();
  uint16_t* GetPtr(string input);
  uint16_t GetRaw(string input);
  int8_t GetRocker(AnalogConfig config);

  void PitchWheel();
  void ModWheel();
  void Dial();
  void DirectPad();  
}  