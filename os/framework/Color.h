#pragma once

#include <stdint.h>
#include "KeyEvent.h"

#define COLOR_LOW_STATE_SCALE 32
#define BLINK_TIME 1000
#define BREATHE_TIME 1000

class Color {
 public:
  uint8_t R = 0;
  uint8_t G = 0;
  uint8_t B = 0;
  uint8_t W = 0;

  Color();
  Color(uint32_t WRGB);
  Color(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nW = 0);

  virtual void Update(){};

  uint32_t RGB(uint8_t brightness = 255);
  uint32_t GRB(uint8_t brightness = 255);
  Color Scale(uint8_t scale);
  Color ToLowBrightness(bool cancel = false, uint8_t scale = COLOR_LOW_STATE_SCALE);  // Helper for UI, make ui variable
                                                                                      // as parameter so the output
                                                                                      // dynamiclly change based on the
                                                                                      // variable
  Color Blink(bool active = true, uint32_t startTime = 0, uint16_t timeLength = BLINK_TIME, uint8_t pwm_high = 1, uint8_t pwm_low = 1);
  Color Blink(KeyInfo keyInfo);
  Color Breathe(bool active = true, uint32_t startTime = 0, uint16_t timeLength = BREATHE_TIME);

  static uint8_t scale8(uint8_t i, uint8_t scale);

  // A special type of scale. It ensures value won't be 0 after the scale.
  static uint8_t scale8_video(uint8_t i, uint8_t scale);

  static Color HsvToRgb(float h, float s, float v);
  static void RgbToHsv(Color rgb, float* h, float* s, float* v);

  operator bool() { return R || G || B || W; }
  bool operator==(const Color& other) const { 
    return R == other.R && G == other.G && B == other.B && W == other.W;
  }
};

const uint8_t led_gamma[256] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,
    2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   5,   5,   5,   5,   6,
    6,   6,   6,   7,   7,   7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,  11,  11,  11,  12,  12,  13,
    13,  13,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  21,  22,  22,  23,  24,
    24,  25,  25,  26,  27,  27,  28,  29,  29,  30,  31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,  39,
    40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  50,  51,  52,  54,  55,  56,  57,  58,  59,  60,  61,
    62,  63,  64,  66,  67,  68,  69,  70,  72,  73,  74,  75,  77,  78,  79,  81,  82,  83,  85,  86,  87,  89,
    90,  92,  93,  95,  96,  98,  99,  101, 102, 104, 105, 107, 109, 110, 112, 114, 115, 117, 119, 120, 122, 124,
    126, 127, 129, 131, 133, 135, 137, 138, 140, 142, 144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167,
    169, 171, 173, 175, 177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213, 215, 218,
    220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255};

const Color COLOR_RED = Color(0xFF0000); //Color(0xFF0000)
const Color COLOR_PINK = Color(0xFF0080); //Color(0xFF0080)
const Color COLOR_ORANGE = Color(0xFF8000); //Color(0xFF8000)
const Color COLOR_YELLOW = Color(0xFFFF00); //Color(0xFFFF00)
const Color COLOR_GREEN = Color(0x80FF00); //Color(0x80FF00)
const Color COLOR_LIME = Color(0x00FF00); //Color(0x00FF00)
const Color COLOR_GUPPIE = Color(0x00FF80); //Color(0x00FF80)
const Color COLOR_CYAN = Color(0x00FFFF); //Color(0x00FFFF)
const Color COLOR_AZURE = Color(0x0080FF); //Color(0x0080FF)
const Color COLOR_BLUE = Color(0x0000FF); //Color(0x0000FF)
const Color COLOR_VIOLET = Color(0x8000FF); //Color(0x8000FF)
const Color COLOR_FUCHSIA = Color(0xFF00FF); //Color(0xFF00FF)
const Color COLOR_WHITE = Color(0xFFFFFF); //Color(0xFFFFFF)
const Color COLOR_BLANK = Color(0x000000); //Color(0x000000)
