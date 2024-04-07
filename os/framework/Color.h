#pragma once

#include <stdint.h>
#include "KeyEvent.h"
#include "Timer.h"

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
  Color Scale(uint8_t value, uint8_t lowest, uint8_t highest, uint8_t brightness = COLOR_LOW_STATE_SCALE);
  Color ToLowBrightness(bool cancel = false, uint8_t scale = COLOR_LOW_STATE_SCALE);  // Helper for UI, make ui variable
                                                                                      // as parameter so the output
                                                                                      // dynamiclly change based on the
                                                                                      // variable
  
  Color Invert();
  Color Contrast(bool clockwise = true);
  Color Rotate(float angle);
  Color Mix(Color color2, float ratio = 0.5);
  Color Blink_Key(KeyInfo keyInfo);
  Color Blink_Color(bool active, Color color);
  Color Blink_Timer(Timer* timer , uint32_t ms);
  Color Blink_Interval(uint32_t ms , Color color, uint32_t start = 0);
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

  bool operator!=(const Color& other) const {
    return R != other.R || G != other.G || B != other.B || W != other.W;
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

const Color COLOR_RED     = Color(0xFF0000); //Color(0xFF0000)
const Color COLOR_ORANGE  = Color(0xFF5500); //Color(0xFF5500)
const Color COLOR_GOLD    = Color(0xFFAA00); //Color(0xFFAA00)
const Color COLOR_YELLOW  = Color(0xFFFF00); //Color(0xFFFF00)
const Color COLOR_GREEN   = Color(0x80FF00); //Color(0x80FF00)
const Color COLOR_LIME    = Color(0x00FF00); //Color(0x00FF00)
const Color COLOR_CYAN    = Color(0x00FF55); //Color(0x00FF55)
const Color COLOR_AZURE   = Color(0x00FFFF); //Color(0x00FFFF)
const Color COLOR_BLUE    = Color(0x0000FF); //Color(0x0000FF)
const Color COLOR_PURPLE  = Color(0x5500FF); //Color(0x5500FF) 
const Color COLOR_VIOLET  = Color(0xAA00FF); //Color(0xAA00FF)
const Color COLOR_PINK    = Color(0xFF00AA); //Color(0xFF00AA)
const Color COLOR_WHITE   = Color(0xFFFFFF); //Color(0xFFFFFF)
const Color COLOR_BLANK   = Color(0x000000); //Color(0x000000)

const Color COLOR_CONFIG[16] = {
  COLOR_RED,   COLOR_ORANGE,    COLOR_GOLD,      COLOR_YELLOW, COLOR_GREEN,  COLOR_LIME,   COLOR_CYAN,      Color(0x00FFAA),
  COLOR_AZURE, Color(0x00AAFF), Color(0x0055FF), COLOR_BLUE,   COLOR_PURPLE, COLOR_VIOLET, Color(0xFF00FF), COLOR_PINK,
};

const Color COLOR_KNOB_8PAGE[8] = {COLOR_RED, COLOR_PINK, COLOR_VIOLET, COLOR_PURPLE, COLOR_ORANGE, COLOR_GOLD, COLOR_YELLOW, COLOR_GREEN};
const Color COLOR_SEQ_4PAGE[4]  = {COLOR_LIME, COLOR_GREEN, COLOR_YELLOW, COLOR_GOLD};

const Color COLOR_PIANO_PAD[2]  = {COLOR_AZURE, COLOR_BLUE}; // [0] ragular color [1] root color
const Color COLOR_NOTE_PAD[2]   = {COLOR_PURPLE, COLOR_PINK}; // [0] ragular color [1] root color
const Color COLOR_DRUM_PAD[2]   = {COLOR_ORANGE, COLOR_RED}; // [0] ragular color [1] root color
