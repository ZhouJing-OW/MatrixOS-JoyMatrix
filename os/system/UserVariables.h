// This file is for user modifiable variables
#pragma once

#include "framework/Framework.h"

#define USER_VAR_NAMESPACE "USER_VAR"
#define UserVar(name, type, default_value) inline CreateSavedVar(USER_VAR_NAMESPACE, name, type, default_value)

namespace MatrixOS::UserVar
{
  // variable name, variable type, variable default
  UserVar(device_id, uint16_t, 0);
  UserVar(rotation, EDirection, TOP);
  UserVar(brightness, uint8_t, 16);
  UserVar(currentBrightness, uint8_t, 16);
  UserVar(velocity_sensitive, bool, false);
  UserVar(pressureSensitive, bool, false);
  UserVar(pressureToVelocity_Default, uint8_t, 127);
  UserVar(pressureToVelocity_Max, uint8_t, 127);
  UserVar(pressureToVelocity_Min, uint8_t, 40);
  
}
