// #pragma once

// #include "UIComponent.h"
// #include "MatrixOS.h"

// class KnobBar : public UIComponent {
//  public:
  
//   Dimension dimension;
//   std::vector<KnobConfig*> knobPt;
//   std::function<void()> callback;
//   int8_t activePoint = 0;

//   KnobBar(){};
//   KnobBar( Dimension dimension, std::vector<KnobConfig> &knob, std::function<void()> callback = nullptr) {
//     Setup(dimension, knob, callback);
//   };

//   virtual void Setup(Dimension dimension, std::vector<KnobConfig> &knob, std::function<void()> callback = nullptr){
//     this->dimension = dimension;
//     this->callback = callback;
//     for(uint8_t n = 0; n < knob.size(); n++){
//       this->knobPt.push_back(&knob[n]);
//     }
//   }

//   virtual bool Callback() {
//     if (callback != nullptr) {
//       callback();
//       return true;
//     }
//     return false;
//   }

//   Color GetColor() { return knobPt[0]->color; }
//   Dimension GetSize() { return dimension; }

//   virtual bool Render(Point origin) {

//     for (uint8_t x = 0; x < dimension.x; x++) 
//     { 
//       for(uint8_t y = 0; y < dimension.y; y++) 
//       { 
//         Point xy = origin + Point(x, 0);
//         uint8_t i = y * dimension.x + x;

//         if(knobPt[i] != nullptr)
//         {
//           int8_t val = knobPt[i]->byte2;
//           Color thisColor = knobPt[i]->color;
          
//           uint8_t LowLight = 16;
//           float ratio = (256 - LowLight) / 2;
//           int8_t range = knobPt[i]->max - knobPt[i]->min;
//           int8_t halfRange = range / 2;

//           if (knobPt[i]->enable == true && i < knobPt.size())
//           { 
//             if (knobPt[i]->def > halfRange - 1 && knobPt[i]->def < halfRange + 1) // The default value is centered
//             {
//                 float hue;
//                 float s;
//                 float v;
//               if (val >= halfRange + 2)
//               {
//                 uint8_t halfVal = val - halfRange < 0 ? 0 : val - halfRange;
//                 uint8_t scale = (halfVal * ratio / halfRange + LowLight) > 255 ? 255 : (halfVal * ratio / halfRange + LowLight);
//                 MatrixOS::LED::SetColor(xy, knobPt[i]->lock ? thisColor.Scale(scale) : thisColor.Scale(scale).Blink(Device::KeyPad::fnState));
//               }
//               else if (val >= halfRange - 2 && val <= halfRange + 2)
//               {
//                 Color::RgbToHsv(thisColor, &hue, &s, &v);
//                 Color tempColor = Color::HsvToRgb(hue, 0.5, 1);
//                 MatrixOS::LED::SetColor(xy, knobPt[i]->lock ? tempColor.Scale(LowLight) : tempColor.Scale(LowLight).Blink(Device::KeyPad::fnState));
//               }
//               else if (val < halfRange - 2)
//               {
//                 Color::RgbToHsv(thisColor, &hue, &s, &v);
//                 if(0.5 - hue > 0)
//                   hue = 0.5 - hue;
//                 else
//                   hue = 1.5 - hue;
//                 Color tempColor = Color::HsvToRgb(hue, s, v);
                
//                 uint8_t scale = (((halfRange - val) * ratio / halfRange + LowLight) > 255 ? 255 : ((halfRange - val) * ratio / halfRange + LowLight));
//                 MatrixOS::LED::SetColor(xy, knobPt[i]->lock ? tempColor.Scale(scale) : tempColor.Scale(scale).Blink(Device::KeyPad::fnState));
//               }
//             }
//             else
//             {
//               uint8_t scale = (val * ratio / range + LowLight) > 255 ? 255 : (val * ratio / range + LowLight);
//               MatrixOS::LED::SetColor(xy, knobPt[i]->lock ? thisColor.Scale(scale) : thisColor.Scale(scale).Blink(Device::KeyPad::fnState));
//             }
//           } else {
//             MatrixOS::LED::SetColor(xy, COLOR_BLANK);
//           }
//         }
//       }
//     }
//     return true;
//   }

//   virtual bool KeyEvent(Point xy, KeyInfo* keyInfo) {

//     uint8_t i = xy.y * dimension.x + xy.x;
//     if (i < knobPt.size() && knobPt[i] != nullptr) {
//       if (keyInfo->state == PRESSED && knobPt[i]->enable == true){
//         if((Device::KeyPad::fnState == ACTIVATED || Device::KeyPad::fnState == HOLD) && knobPt[i]->lock == false) {
//           MatrixOS::Component::Knob_Setting(knobPt[i], false);
//           return true;
//         }
//       }

//       if (keyInfo->state == HOLD && knobPt[i]->enable == true)
//       {
//         knobPt[i]->byte2 = knobPt[i]->def;
//         MatrixOS::Component::Knob_Function(knobPt[i]);
//       };
//       return true;
//     }
//     return false;
//   }
// };
