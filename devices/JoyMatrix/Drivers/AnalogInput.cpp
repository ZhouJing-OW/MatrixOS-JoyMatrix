#include "MatrixOS.h"
#include "Device.h"
#include "timers.h"
#include "esp_adc\adc_oneshot.h"
#include <functional>
#include <map>
#include <math.h>

#define ANALOG_INPUT_ADC_ATTEN ADC_ATTEN_DB_11
#define ANALOG_INPUT_ADC_WIDTH ADC_BITWIDTH_12

extern const uint8_t ulp_analog_input_start[] asm("_ulp_analog_input_start");
extern const uint8_t ulp_analog_input_end[] asm("_ulp_analog_input_end");

namespace Device::AnalogInput
{
  StaticTimer_t analog_input_timer_def;
  TimerHandle_t analog_input_timer;
  
  uint16_t  reading[7];
  uint16_t  readData[7];

  uint16_t  lastPitch = 0x1FFE;
  int8_t    lastMod = 0;

  std::unordered_map<uint16_t, KnobConfig*> dialPtr;
  vector<uint16_t> keyID;

  int8_t    dial_max;
  int8_t    dial_min;
  bool      dialMode = true;
  bool      dialActive_L = false;
  bool      dialActive_R = false;
  float     anglePrev_L;
  float     anglePrev_R;  
  std::function<void()> dial_callback;

  const uint8_t dialDevide = 24;

  int8_t*   upDown = nullptr;
  int8_t    upDown_max;
  int8_t    upDown_min;
  bool      upDown_loop;
  uint32_t  upDown_time = 0; 
  std::function<void()> upDown_callback;

  int8_t*   leftRight = nullptr;
  int8_t    leftRight_max;
  int8_t    leftRight_min;
  bool      leftRight_loop;
  uint32_t  leftRight_time = 0;
  std::function<void()> leftRight_callback;

  const uint16_t timeInterval = 200;

  // Config analog inputs
  const adc_unit_t L_rocker_x_unit = ADC_UNIT_1;
  const adc_channel_t L_rocker_x_channel = ADC_CHANNEL_1;//IO1
  const string L_rocker_x_direction = "Negative";

  const adc_unit_t L_rocker_y_unit = ADC_UNIT_1;
  const adc_channel_t L_rocker_y_channel = ADC_CHANNEL_0;//IO2
  const string L_rocker_y_direction = "Negative";

  const adc_unit_t R_rocker_x_unit = ADC_UNIT_2;
  const adc_channel_t R_rocker_x_channel = ADC_CHANNEL_0;//IO12
  const string R_rocker_x_direction = "Positive";

  const adc_unit_t R_rocker_y_unit = ADC_UNIT_2;
  const adc_channel_t R_rocker_y_channel = ADC_CHANNEL_1;//IO11
  const string R_rocker_y_direction = "Positive";

  const adc_unit_t L_pressure_unit = ADC_UNIT_2;
  const adc_channel_t L_pressure_channel = ADC_CHANNEL_3;//IO14
  const string L_pressure_direction = "Positive";

  const adc_unit_t R_pressure_unit = ADC_UNIT_2;
  const adc_channel_t R_pressure_channel = ADC_CHANNEL_2;//IO13
  const string R_pressure_direction = "Negative";

  const adc_unit_t B_fader_unit = ADC_UNIT_1;
  const adc_channel_t B_fader_channel = ADC_CHANNEL_9;//IO10
  const string B_fader_direction = "Positive";

  adc_unit_t analog_input_unit[7]
    {L_rocker_x_unit, L_rocker_y_unit, R_rocker_x_unit, R_rocker_y_unit, L_pressure_unit, R_pressure_unit, B_fader_unit};

  adc_channel_t analog_input_channel[7]
    {L_rocker_x_channel, L_rocker_y_channel, R_rocker_x_channel, R_rocker_y_channel, L_pressure_channel, R_pressure_channel, B_fader_channel};

  string analog_input_direction[7]
    {L_rocker_x_direction, L_rocker_y_direction, R_rocker_x_direction, R_rocker_y_direction, L_pressure_direction, R_pressure_direction, B_fader_direction};

  adc_oneshot_unit_handle_t adc_handle[2];

  inline AnalogConfig LX = { .name = "LX",  .max = 2900,  .min = 800,   .middle = 1870, };

  inline AnalogConfig LY = { .name = "LY",  .max = 2950,  .min = 930,   .middle = 1970, };

  inline AnalogConfig RX = { .name = "RX",  .max = 3250,  .min = 950,   .middle = 2150, };

  inline AnalogConfig RY = { .name = "RY",  .max = 2950,  .min = 1050,  .middle = 1880, };

  inline AnalogConfig LP = { .name = "LP",  .max = 2950,  .min = 1050,  .middle = 1880, };

  inline AnalogConfig RP = { .name = "RP",  .max = 2950,  .min = 1050,  .middle = 1880, };

  inline AnalogConfig BF = { .name = "BF",  .max = 2950,  .min = 1050,  .middle = 1880, };

  void Init() {
    adc_oneshot_unit_init_cfg_t init_config[2];

    init_config[0] = {.unit_id = ADC_UNIT_1, .clk_src = ADC_RTC_CLK_SRC_DEFAULT, .ulp_mode = ADC_ULP_MODE_DISABLE};

    init_config[1] = {.unit_id = ADC_UNIT_2, .clk_src = ADC_RTC_CLK_SRC_DEFAULT, .ulp_mode = ADC_ULP_MODE_DISABLE};

    adc_oneshot_new_unit(&init_config[0], &adc_handle[0]);
    adc_oneshot_new_unit(&init_config[1], &adc_handle[1]);

    adc_oneshot_chan_cfg_t adc_config = {
        .atten = ANALOG_INPUT_ADC_ATTEN,
        .bitwidth = ANALOG_INPUT_ADC_WIDTH,
    };

    for (uint8_t i = 0; i < 7; i++)
    {
      adc_oneshot_config_channel((analog_input_unit[i] == ADC_UNIT_1) ? adc_handle[0] : adc_handle[1], analog_input_channel[i], &adc_config);
    }

    analog_input_timer =
        xTimerCreateStatic(NULL, configTICK_RATE_HZ / Device::analog_input_scanrate, true, NULL, reinterpret_cast<TimerCallbackFunction_t>(Scan), &analog_input_timer_def);
    xTimerStart(analog_input_timer, 0);
    
  }

  uint16_t middleOfThree(uint16_t a, uint16_t b, uint16_t c) {
    // Checking for a
    if ((b <= a && a <= c) || (c <= a && a <= b))
      return a;

    // Checking for b
    if ((a <= b && b <= c) || (c <= b && b <= a))
      return b;

    return c;
  }

  uint16_t minOfThree(uint16_t a, uint16_t b, uint16_t c) {
    // Checking for a
    if (a < b && a < c)
      return a;

    // Checking for b
    if (b < a && b < c)
      return b;

    return c;
  }

  void Scan() {
    
    for(uint8_t i = 0; i < 7; i++)
    {
      int result[3];

      for(uint8_t j = 0; j < 3; j++){
        adc_oneshot_read((analog_input_unit[i] == ADC_UNIT_1 ? adc_handle[0] : adc_handle[1]), analog_input_channel[i], &result[j]);
      }
      
      if(analog_input_direction[i] == "Negative") reading[i] = 0b0000111111111111 - middleOfThree(result[0], result[1], result[2]);
      else reading[i] = middleOfThree(result[0], result[1], result[2]);
    }

    if(dialPtr.size() > 0) dialMode = true;
    else {
      dialMode = false;
      dial_callback = nullptr;
    }
    
    void PitchWheel();
    void ModWheel();
    void DirectPad();  
    void Dial();

    PitchWheel();
    ModWheel();
    DirectPad();
    Dial();
  }

  void PitchWheel()
  {
    int8_t lx = GetRocker(LX);
    int8_t ly = GetRocker(LY);
    int16_t change = ly * 64 + lx * 5.3;
    if (change > 0x1FFE) change = 0x1FFE;
    if (change < - 0x1FFE) change = - 0x1FFE;

    uint16_t pitch = 0x1FFF + change;
    if (pitch != lastPitch){
      int8_t channel = MatrixOS::UserVar::global_MIDI_CH;
      //MLOGD("Pitch Wheel", "lx:%d, ly:%d, pitch:%d, ch:%d", lx, ly, pitch, channel);
      lastPitch = pitch;
      MatrixOS::MIDI::Send(MidiPacket(0, PitchChange, channel, pitch));
    }
  }

  void ModWheel()
  {
    if(!dialMode)
    {
      uint8_t mod = GetRaw("BF") / 32;
      if (lastMod != mod){
        int8_t channel = MatrixOS::UserVar::global_MIDI_CH;
        // MLOGD("Mod Wheel", "mod:%d, ch:%d", mod, channel);
        lastMod = mod;
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, channel, 1, mod));
      }
    }
  }

  void DirectPad()
  {
    // if (GetRocker(RY) != 0) MLOGD("Rocker","ry = %d", GetRocker(RY));
    // if (GetRocker(RX) != 0) MLOGD("Rocker","rx = %d", GetRocker(RX));
    if(!dialMode)
    {
      if(upDown != nullptr && upDown_max > upDown_min)
      {
        if ((GetRocker(RY) > 64) && (upDown_time < MatrixOS::SYS::Millis()))
        {
          *upDown = (*upDown + 1 <= upDown_max) ? *upDown + 1 : (upDown_loop ? upDown_min : upDown_max);
          upDown_time = MatrixOS::SYS::Millis() + timeInterval;
          if(upDown_callback != nullptr) upDown_callback();
          // MLOGD("DirectPad", "_up: %d", *upDown);
        } else if ((GetRocker(RY) < -64) && (upDown_time < MatrixOS::SYS::Millis()))
        {
          *upDown = (*upDown - 1 >= upDown_min) ? *upDown - 1 : (upDown_loop ? upDown_max : upDown_min);
          upDown_time = MatrixOS::SYS::Millis() + timeInterval;
          // MLOGD("DirectPad", "_down: %d", *upDown);
        } else if (GetRocker(RY) > -16 && (GetRocker(RY) < 16)) upDown_time = 0;
      }

      if(leftRight != nullptr && leftRight_max > leftRight_min)
      {
        if ((GetRocker(RX) > 64) && (leftRight_time < MatrixOS::SYS::Millis()))
        {
          *leftRight = (*leftRight + 1 <= leftRight_max) ? *leftRight + 1 : (leftRight_loop ? leftRight_min : leftRight_max);
          leftRight_time = MatrixOS::SYS::Millis() + timeInterval;
          if(leftRight_callback != nullptr) leftRight_callback();
          // MLOGD("DirectPad", "_left: %d", *leftRight);
        }else if ((GetRocker(RX) < -64) && (leftRight_time < MatrixOS::SYS::Millis()))
        {
          *leftRight = (*leftRight - 1 >= leftRight_min) ? *leftRight - 1 : (leftRight_loop ? leftRight_max : leftRight_min);
          leftRight_time = MatrixOS::SYS::Millis() + timeInterval;
          if(leftRight_callback != nullptr) leftRight_callback();
          // MLOGD("DirectPad", "_right: %d", *leftRight);
        } else if (GetRocker(RX) > -16 && (GetRocker(RX) < 16)) leftRight_time = 0;
      }
    }
  }

  void Dial()
  {
    if(dialMode)
    {
      int8_t rockerR_y = GetRocker(RY); int8_t rockerR_x = GetRocker(RX);
      int8_t rockerL_y = GetRocker(LY); int8_t rockerL_x = GetRocker(LX);
      uint8_t r_R = sqrt(pow(rockerR_y, 2) + pow(rockerR_x, 2));
      uint8_t r_L = sqrt(pow(rockerL_y, 2) + pow(rockerL_x, 2));
      float angle = 0;
      float* anglePrev;
      int8_t delta = 0;

      if(r_R > 16 && dialActive_L == false)
      { 
        angle = atan2(rockerR_y, rockerR_x);
        if(dialActive_R == false) 
        {
          anglePrev_R = angle;
          anglePrev = &anglePrev_R;
          dialActive_R = true;
        }
      } else if (r_R <= 16) dialActive_R = false;
      
      if(r_L > 16 && dialActive_R == false)
      {
        angle = atan2(rockerL_y, rockerL_x);
        if(dialActive_L == false) 
        { 
          anglePrev_L = angle;
          anglePrev = &anglePrev_L;
          dialActive_L = true;
        }
      } else if (r_L <= 16) dialActive_L = false;

      if(dialActive_R || dialActive_L){
        float angleDelta = angle - *anglePrev;
        if(angleDelta > M_PI) angleDelta = angleDelta - 2 * M_PI;
        if(angleDelta < -M_PI) angleDelta = angleDelta + 2 * M_PI;
        if ((angleDelta > M_PI / dialDevide && angleDelta < M_PI) || (angleDelta < -M_PI / dialDevide && angleDelta > -M_PI))
        {
          delta = angleDelta / (M_PI / dialDevide);
          if (delta > dialDevide / 2) delta = dialDevide - delta; else if (delta < -8) delta = -dialDevide - delta;
          *anglePrev = angle - (angleDelta - delta * (M_PI / dialDevide));
          if (*anglePrev - M_PI > 0) *anglePrev = -(*anglePrev - M_PI);
          else 
          if (*anglePrev + M_PI < 0) *anglePrev = -(*anglePrev + M_PI);
          // MLOGD("Dial", " angle = %f ,anglePrev = %f  delta = %d", angle, *anglePrev, -delta);
        }
      } 

      
      for (auto it = dialPtr.begin(); it != dialPtr.end(); it++){

        KeyInfo* keyInfo = Device::KeyPad::GetKey(it->first);
        if(keyInfo->state == ACTIVATED || keyInfo->state == HOLD)
        { 
          if(it->second->byte2 != it->second->def && Device::KeyPad::ShiftActived())
          {
            uint16_t ms = 200;
            uint8_t step = 127 / (ms / (1000 / Device::analog_input_scanrate));
            int8_t i = it->first;
            int8_t target = it->second->def;
            if (it->second->byte2 < target) it->second->byte2 = it->second->byte2 + step > target ? target : it->second->byte2 + step;
            if (it->second->byte2 > target) it->second->byte2 = it->second->byte2 - step < target ? target : it->second->byte2 - step;
            MatrixOS::Component::Knob_Function(it->second);
          } 

          if (delta != 0)
          {
            if (it->second->byte2 - delta > it->second->max) it->second->byte2 = it->second->max;
            else if (it->second->byte2 - delta < it->second->min) it->second->byte2 = it->second->min;
            else it->second->byte2 -= delta;
            MatrixOS::Component::Knob_Function(it->second);
          }
        }

        if(keyInfo->state == RELEASED || keyInfo->state == IDLE)
        {
          keyID.push_back(it->first);
        }
      }
      
      for (uint8_t i = 0; i < keyID.size(); i++)
      {
        auto it = dialPtr.find(keyID[i]);
        dialPtr.erase(it);
      }
      
      keyID.clear();
    }
  }

  void UseDial (Point xy, KnobConfig* knob, std::function<void()> callback){
    uint16_t ID = Device::KeyPad::XY2ID(xy);
    dial_callback = callback;
    dialPtr.emplace(ID, knob);
  }

  uint16_t* GetPtr(string input)
  {
    if (input == "LX") {return &reading[0];}      // left X
    else if (input == "LY") {return &reading[1];} // left Y
    else if (input == "RX") {return &reading[2];} // right X
    else if (input == "RY") {return &reading[3];} // right Y
    else if (input == "LP") {return &reading[4];} // left pressure
    else if (input == "RP") {return &reading[5];} // right pressure
    else if (input == "BF") {return &reading[6];} // bottom fader
    else return NULL;
    
  }

  uint16_t GetRaw(string input)
  {
    if (input == "LX") {return reading[0];}      // left X
    else if (input == "LY") {return reading[1];} // left Y
    else if (input == "RX") {return reading[2];} // right X
    else if (input == "RY") {return reading[3];} // right Y
    else if (input == "LP") {return reading[4];} // left pressure
    else if (input == "RP") {return reading[5];} // right pressure
    else if (input == "BF") {return reading[6];} // bottom fader
    else return 0;
  }

  int8_t GetRocker(AnalogConfig config)
  {
    uint16_t raw = GetRaw(config.name);
    if (raw <= config.min) {
      return -127;
    } else if ((raw > config.min) && (raw < config.middle - config.deadZone)) {
      return - ((config.middle - config.deadZone - raw) * 127 / (config.middle - config.deadZone - config.min));
    } else if ((raw >= config.middle - config.deadZone) && (raw <= config.middle + config.deadZone)){
      return 0;
    } else if ((raw > config.middle + config.deadZone) && (raw < config.max)){
      return (raw - (config.middle + config.deadZone)) * 127 / (config.max - (config.middle + config.deadZone));
    } else return 127;
  }

  void SetUpDown(int8_t* up_down, int8_t max, int8_t min, bool loop, std::function<void()> callback){
    upDown = up_down;
    upDown_max = max;
    upDown_min = min;
    upDown_loop = loop;
    upDown_callback = callback;
  }

  void SetLeftRight(int8_t* left_right, int8_t max, int8_t min, bool loop, std::function<void()> callback){
    leftRight = left_right;
    leftRight_max = max;
    leftRight_min = min;
    leftRight_loop = loop;
    leftRight_callback = callback;
  }

}