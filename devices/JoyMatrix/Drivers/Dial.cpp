#include "MatrixOS.h"
#include "Device.h"
#include "AnalogInput.h"
#include <math.h>
#include <functional>
#include <map>
#include <vector>

namespace Device::AnalogInput
{
  extern AnalogConfig LX;
  extern AnalogConfig LY;
  extern AnalogConfig RX;
  extern AnalogConfig RY;

  std::map<uint16_t, KnobConfig*> dialPtr;
  std::map<uint16_t, int16_t> dialValue;
  std::function<void()> dial_callback;

  int8_t dial_max;
  int8_t dial_min;
  bool dialActive_L = false;
  bool dialActive_R = false;
  double anglePrev_L;
  double anglePrev_R;
  double deltaRemain;

  const uint8_t dialDevide = 36;

  void Dial() {
    
    int8_t rockerR_y = GetRocker(RY);
    int8_t rockerR_x = GetRocker(RX);
    int8_t rockerL_y = GetRocker(LY);
    int8_t rockerL_x = GetRocker(LX);
    uint8_t r_R = sqrt(pow(rockerR_y, 2) + pow(rockerR_x, 2));
    uint8_t r_L = sqrt(pow(rockerL_y, 2) + pow(rockerL_x, 2));
    double angle = 0;
    double* anglePrev;
    int8_t delta = 0;

    if (r_R > 16 && dialActive_L == false)
    {
      angle = atan2(rockerR_y, rockerR_x);
      if (dialActive_R == false)
      {
        anglePrev_R = angle;
        anglePrev = &anglePrev_R;
        deltaRemain = 0;
        dialActive_R = true;
      }
    }
    else if (r_R <= 16)
      dialActive_R = false;

    if (r_L > 16 && dialActive_R == false)
    {
      angle = atan2(rockerL_y, rockerL_x);
      if (dialActive_L == false)
      {
        anglePrev_L = angle;
        anglePrev = &anglePrev_L;
        deltaRemain = 0;
        dialActive_L = true;
      }
    }
    else if (r_L <= 16)
      dialActive_L = false;

    if (dialActive_R || dialActive_L)
    {
      double d1 = angle - *anglePrev;
      double d2 = 2 * M_PI - fabs(d1);
      if (d1 > 0)
        d2 *= -1.0;
      double angleDelta = (fabs(d1) < fabs(d2)) ? d1 : d2;
      delta = (int)(angleDelta / (M_PI / dialDevide) + deltaRemain);
      deltaRemain = (int)(angleDelta / (M_PI / dialDevide) + deltaRemain) - delta;
      if (delta != 0)
        *anglePrev = angle;
      // MLOGD("Dial", " angle = %f ,anglePrev = %f  delta = %d", angle, *anglePrev, -delta);
    }

    for (auto it = dialPtr.begin(); it != dialPtr.end(); it++)
    {
      if (it->second->GetPtr() == nullptr) continue;

      KeyInfo* keyInfo = Device::KeyPad::GetKey(it->first);
      if (keyInfo->active())
      {
        if (it->second->Value() != it->second->def && Device::KeyPad::Shift())
        {
          uint16_t ms = 150;
          uint8_t step = 127 / (ms / (1000 / Device::analog_input_scanrate));
          int16_t target = it->second->def;
          if (it->second->Value() < target)
            it->second->SetValue(it->second->Value() + step > target ? target : it->second->Value() + step);
          if (it->second->Value() > target)
            it->second->SetValue(it->second->Value() - step < target ? target : it->second->Value() - step);
          MatrixOS::KnobCenter::Knob_Function(it->second);
        }
        if (delta != 0)
        {
          int16_t* valptr;
          int16_t tempMax;
          int16_t tempMin;
          int16_t byte2Prv = it->second->Value();
          if (it->second->max - it->second->min > 127)
          {
            valptr = it->second->GetPtr();
            tempMax = it->second->max;
            tempMin = it->second->min;
          }
          else
          {
            valptr = &dialValue.find(it->first)->second;
            tempMax = 127;
            tempMin = 0;
          }
            
          if (*valptr - delta > tempMax)
            *valptr = tempMax;
          else if (*valptr - delta < tempMin)
            *valptr = tempMin;
          else
            *valptr -= delta;

          if (valptr == &dialValue.find(it->first)->second)
            it->second->SetValue(dialValue.find(it->first)->second * (it->second->max - it->second->min) / 127 + it->second->min);
          if (byte2Prv != it->second->Value())
            MatrixOS::KnobCenter::Knob_Function(it->second);
        }
      }
      if (keyInfo->state == RELEASED || keyInfo->state == IDLE)
      {
        dialValue.erase(it->first);
        dialPtr.erase(it);
        return;
      }
      // MLOGD("Dial", "keyID = %d, value = %d", it->first, it->second->Value());
    }
  }

  void UseDial(Point xy, KnobConfig* knob, std::function<void()> callback) {
    uint16_t keyID = Device::KeyPad::XY2ID(xy);
    dial_callback = callback;
    dialPtr.emplace(keyID, knob);
    int16_t valueMap = (knob->Value() - knob->min) * 127 / (knob->max - knob->min);
    dialValue.emplace(keyID, valueMap);
  }

  void DisableDial()
  {
    dialValue.clear();
    dialPtr.clear();
  }

  KnobConfig* GetDialPtr() {
    if (!dialPtr.empty()) return dialPtr.begin()->second;
    else return nullptr;
  }

}