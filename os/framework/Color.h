#pragma once

#include <stdint.h>
#include "KeyEvent.h"
#include "Timer.h"

#define COLOR_LOW_STATE_SCALE 32
#define BLINK_TIME 1000
#define BREATHE_TIME 1000

enum ColorLabel : uint32_t {
  WHITE     = 0xFFFFFF,  //0xFFFFFF
  BLANK     = 0x000000,  //0x000000

  RED       = 0xFF0000,  //0xFF0000
  TOMATO    = 0xFF2A00,  //0xFF2A00
  ORANGE    = 0xFF5500,  //0xFF5500
  GOLD      = 0xFFAA00,  //0xFFAA00
  YELLOW    = 0xFFFF00,  //0xFFFF00
  LAWN      = 0x55FF00,  //0x55FF00
  GREEN     = 0x00FF00,  //0x00FF00
  TURQUOISE = 0x00FF2A,  //0x00FF2A
  CYAN      = 0x00FFFF,  //0x00FFFF
  DEEPSKY   = 0x002AFF,  //0x002AFF
  BLUE      = 0x0000FF,  //0x0000FF
  PURPLE    = 0x2A00FF,  //0x2A00FF
  VIOLET    = 0x8000FF,  //0x8000FF
  MAGENTA   = 0xFF00FF,  //0xFF00FF
  DEEPPINK  = 0xFF0080,  //0xFF0080
  MAROON    = 0xFF002A,  //0xFF002A

  RED_LS         = 0xE01010,
  TOMATO_LS      = 0xE03010,
  ORANGE_LS      = 0xE06010, 
  GOLD_LS        = 0xE09010,
  YELLOW_LS      = 0xE0E010,
  LAWN_LS        = 0x50E010,
  GREEN_LS       = 0x10E010,
  TURQUOISE_LS   = 0x10E050,
  CYAN_LS        = 0x10E0E0,
  DEEPSKY_LS     = 0x1080E0,
  BLUE_LS        = 0x1010E0,
  PURPLE_LS      = 0x5010E0,
  VIOLET_LS      = 0x9010E0,
  MAGENTA_LS     = 0xE010E0,
  DEEPPINK_LS    = 0xE01070,
  MAROON_LS      = 0xE01030,

  RED_HL         = 0xFF4040,
  TOMATO_HL      = 0xFF6040,
  ORANGE_HL      = 0xFF8040, 
  GOLD_HL        = 0xFFC040,
  YELLOW_HL      = 0xFFFF40,
  LAWN_HL        = 0x80FF40,
  GREEN_HL       = 0x40FF40,
  TURQUOISE_HL   = 0x40FF80,
  CYAN_HL        = 0x40FFFF,
  DEEPSKY_HL     = 0x4080FF,
  BLUE_HL        = 0x4040FF,
  PURPLE_HL      = 0x6040FF,
  VIOLET_HL      = 0xA040FF,
  MAGENTA_HL     = 0xFF40FF,
  DEEPPINK_HL    = 0xFF40A0,
  MAROON_HL      = 0xFF4060,

};

class Color {
 public:
  uint8_t R = 0;
  uint8_t G = 0;
  uint8_t B = 0;
  uint8_t W = 0;

  Color();
  Color(uint32_t WRGB);
  Color(ColorLabel label);
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

static const ColorLabel COLOR_HIGH_S[16] = {
  RED,          TOMATO,       ORANGE,       GOLD,         YELLOW,       LAWN,         GREEN,        TURQUOISE,
  CYAN,         DEEPSKY,      BLUE,         PURPLE,       VIOLET,       MAGENTA,      DEEPPINK,     MAROON,
};

static const  ColorLabel COLOR_LOW_S[16] = {
  RED_LS,       TOMATO_LS,    ORANGE_LS,    GOLD_LS,      YELLOW_LS,    LAWN_LS,      GREEN_LS,     TURQUOISE_LS,
  CYAN_LS,      DEEPSKY_LS,   BLUE_LS,      PURPLE_LS,    VIOLET_LS,    MAGENTA_LS,   DEEPPINK_LS,  MAROON_LS,
};

static const  ColorLabel COLOR_HIGH_L[16] = {
  RED_HL,       TOMATO_HL,    ORANGE_HL,    GOLD_HL,      YELLOW_HL,    LAWN_HL,      GREEN_HL,     TURQUOISE_HL,
  CYAN_HL,      DEEPSKY_HL,   BLUE_HL,      PURPLE_HL,    VIOLET_HL,    MAGENTA_HL,   DEEPPINK_HL,  MAROON_HL,  
};

static const  ColorLabel COLOR_KNOB_8PAGE [8] = { RED,          DEEPPINK,     VIOLET,       PURPLE,       ORANGE,       GOLD,         YELLOW,       LAWN};

static const  ColorLabel COLOR_PIANO_PAD  [2] = { DEEPSKY_LS,   DEEPSKY};  // [0] ragular color [1] root color
static const  ColorLabel COLOR_NOTE_PAD   [2] = { PURPLE_LS,    PURPLE};  // [0] ragular color [1] root color
static const  ColorLabel COLOR_DRUM_PAD   [2] = { TOMATO_LS,    TOMATO};    // [0] ragular color [1] root color
