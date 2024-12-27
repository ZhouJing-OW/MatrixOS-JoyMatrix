#pragma once

// SYSTEM_APPLICATION
#include "applications/Shell/Shell.h"
#include "applications/Performance/Performance.h"

// USER APPLICATION
#include "applications/Drambo/Drambo.h"
// #include "applications/MainPad/MainPad.h"
// #include "applications/Lighting/Lighting.h"

// BOOT ANIMATION
#include "applications/Matrix/MatrixBoot/MatrixBoot.h"

// DEVICE APPLICATION
#include "applications/Matrix/FactoryMenu/FactoryMenu.h"

#define OS_SHELL APPID("203 Electronics", "Shell")

#define DEFAULT_BOOTANIMATION APPID("203 Electronics", "Matrix Boot")