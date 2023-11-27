#include "BrightnessControl.h"
#include "UITwoToneSelector.h"
#include "UI4pxNumberWithColorFunc.h"

void BrightnessControl::Start() {
  name = "Brightness Control";
  nameColor = Color(0xFFFFFF);
  
  // Brightness Control
  threshold = Device::brightness_level[sizeof(Device::brightness_level) / sizeof(Device::brightness_level[0]) - 1]; //Get the last element
#ifndef FINE_LED_BRIGHTNESS
  map = Device::brightness_level;
  map_length = sizeof(Device::brightness_level) / sizeof(Device::brightness_level[0]);
#else
  map = Device::fine_brightness_level;
  map_length = sizeof(Device::fine_brightness_level) / sizeof(Device::fine_brightness_level[0]);
#endif

  Dimension dimension = Dimension(map_length / 4 + bool(map_length % 4), 4);

  float multiplier = (float)threshold / 100;
  int32_t displayValue = ((float)MatrixOS::UserVar::brightness / multiplier);

  UITwoToneSelector brightnessSelector(dimension, map_length, Color(0xFFFFFF), Color(0xFF0000), (uint8_t*)&MatrixOS::UserVar::brightness.value, threshold, map,
                                       [&](uint8_t value) -> void {
                                         MatrixOS::UserVar::brightness.Set(value);
                                         displayValue = value / multiplier;
                                       });
  AddUIComponent(brightnessSelector,
                 Point(0, 0));

  // Number Display
  UI4pxNumberWithColorFunc numDisplay([&]() -> Color { return (MatrixOS::UserVar::brightness.value > threshold) ? Color(0xFF0000) : Color(0x00FF00); }, 3,
                                      (int32_t*)&displayValue, (MatrixOS::UserVar::brightness.value > threshold) ? Color(0xFF0000) : Color(0x00FF00));
  AddUIComponent(numDisplay, Point(5, 0));

  UI::Start();
}
