#include "MatrixOS.h"
#include "Color.h"
#include <cmath>

float fract(float x) { return x - int(x); }

float mix(float a, float b, float t) { return a + (b - a) * t; }

float step(float e, float x) { return x < e ? 0.0 : 1.0; }


#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

Color::Color() {
  W = 0;
  R = 0;
  G = 0;
  B = 0;
}

Color::Color(uint32_t WRGB) {
  W = (WRGB & 0xFF000000) >> 24;
  R = (WRGB & 0x00FF0000) >> 16;
  G = (WRGB & 0x0000FF00) >> 8;
  B = (WRGB & 0x000000FF);
}

Color::Color(ColorLabel label) {
  W = (label & 0xFF000000) >> 24;
  R = (label & 0x00FF0000) >> 16;
  G = (label & 0x0000FF00) >> 8;
  B = (label & 0x000000FF);
}

Color::Color(uint8_t nR, uint8_t nG, uint8_t nB, uint8_t nW) {
  R = nR;
  G = nG;
  B = nB;
  W = nW;
}

uint32_t Color::RGB(uint8_t brightness) {
  if (brightness != 255)
    return (scale8_video(R, brightness) << 16) | (scale8_video(G, brightness) << 8) | scale8_video(B, brightness); // Use scale_video to ensure it doesn't get completely removed
  return (R << 16) | (G << 8) | B;
}

uint32_t Color::GRB(uint8_t brightness) {
  if (brightness != 255)
    return (scale8_video(G, brightness) << 16) | (scale8_video(R, brightness) << 8) | scale8_video(B, brightness); // Use scale_video to ensure it doesn't get completely removed
  return (G << 16) | (R << 8) | B;
}

Color Color::Scale(uint8_t brightness) {
  return Color(scale8_video(R, brightness), scale8_video(G, brightness), scale8_video(B, brightness)); // Use scale_video to ensure it doesn't get completely removed
}

Color Color::Scale(uint8_t value, uint8_t lowest, uint8_t highest, uint8_t brightness) {
  uint16_t scale = (255 - brightness) * (float)(value - lowest) / (float)(highest - lowest) + brightness;
  scale = scale > 255 ? 255 : scale;
  return Color(scale8_video(R, scale), scale8_video(G, scale), scale8_video(B, scale)); // Use scale_video to ensure it doesn't get completely removed
}

Color Color::Dim(uint8_t scale) {
  return Scale(scale); 
}

Color Color::DimIfNot(bool not_dim, uint8_t scale) {
  if (!not_dim)
  { return Scale(scale); }
  return Color(R, G, B, W);
}

Color Color::Gamma() {
  return Color(led_gamma[R], led_gamma[G], led_gamma[B]);
}

Color Color::Mix(Color color2, float ratio) {
  return Color((uint8_t)mix((float)R , color2.R, ratio), (uint8_t)mix((float)G , color2.G, ratio), (uint8_t)mix((float)B , color2.B, ratio));
}

Color Color::Invert() {
  return Color(255 - R, 255 - G, 255 - B);
}

Color Color::Contrast(bool clockwise) {
  return clockwise? Color(B, R, G) : Color(G, B, R);
}

Color Color::Rotate(float angle) {
  float h = 0; float s = 0; float v = 0;
  RgbToHsv(Color (R, G, B, W), &h, &s, &v);
  angle = fract(angle / 360);
  h = fract(h + angle);
  return Color::HsvToRgb(h, s, v);
}

Color Color::Blink_Timer(Timer* timer, uint32_t ms)
{
  bool cancel = timer->IsLonger(100);
  if (!cancel)
    return Color(WHITE);
  else
    return Color(R, G, B, W);
}

Color Color::Blink_Interval(uint32_t ms, Color color, uint32_t start)
{
  bool cancel = ((MatrixOS::SYS::Millis() - start) / (ms / 2)) % 2;
  if (!cancel)
    return color;
  else
    return Color(R, G, B, W);
}

Color Color::Blink_Key(KeyInfo key) {
  if (key == ACTIVATED || key == HOLD)
  {
    bool cancel = ((MatrixOS::SYS::Millis()) / (BLINK_TIME / 2)) % 2;
    if (!cancel)
      return Color(scale8(R, COLOR_LOW_STATE_SCALE), scale8(G, COLOR_LOW_STATE_SCALE), scale8(B, COLOR_LOW_STATE_SCALE));
  }
  return Color(R, G, B, W);
}

Color Color::Blink_Color(bool active, Color color) {
  if(active) {
    bool cancel = ((MatrixOS::SYS::Millis()) / (BLINK_TIME / 2)) % 2;
    if (!cancel) return color;
  }
  return Color(R, G, B, W);
}

uint8_t Color::scale8(uint8_t i, uint8_t scale) {
  return ((uint16_t)i * (uint16_t)scale) >> 8;
}

uint8_t Color::scale8_video(uint8_t i, uint8_t scale) {
  return (((uint16_t)i * (uint16_t)scale) >> 8) + ((i && scale) ? 1 : 0);
}

Color Color::HsvToRgb(float h, float s, float v) {
  uint8_t r = int(255 * v * mix(1.0, constrain(std::abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s));
  uint8_t g = int(255 * v * mix(1.0, constrain(std::abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s));
  uint8_t b = int(255 * v * mix(1.0, constrain(std::abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s));
  return Color(r, g, b);
}

void Color::RgbToHsv(Color rgb, float* h, float* s, float* v)
{
  float r = rgb.R / 255.0;
  float g = rgb.G / 255.0;
  float b = rgb.B / 255.0;

  float max = std::max(r, std::max(g, b));
  float min = std::min(r, std::min(g, b));

  *v = max;
  float delta = max - min;
  if (max != 0)
    *s = delta / max;
  else
  {
    // r = g = b = 0
    *s = 0;
    *h = -1;
    return;
  }
  if (r == max)
    *h = (g - b) / delta; // between yellow & magenta
  else if (g == max)
    *h = 2 + (b - r) / delta; // between cyan & yellow
  else
    *h = 4 + (r - g) / delta; // between magenta & cyan
  *h *= 1.0/6; // degrees
  if (*h < 0)
    *h += 1.0;
}

Color Color::Crossfade(Color color1, Color color2, Fract16 ratio) {
  uint8_t r = ratio.to8bits();
  uint8_t newR = (color1.R * (255 - r) + color2.R * r) >> 8;
  uint8_t newG = (color1.G * (255 - r) + color2.G * r) >> 8;
  uint8_t newB = (color1.B * (255 - r) + color2.B * r) >> 8;
  return Color(newR, newG, newB);
}