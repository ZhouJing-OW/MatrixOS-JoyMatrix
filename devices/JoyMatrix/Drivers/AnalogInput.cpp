#include "AnalogInput.h"
#include "esp_adc\adc_oneshot.h"
#include <map>
#include <vector>
#include <functional>

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

  bool      dialMode = false;

  extern std::map<uint16_t, KnobConfig*> dialPtr;
  extern vector<uint16_t> keyID;
  extern std::function<void()> dial_callback;

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

  adc_unit_t analog_input_unit[7] =
    {L_rocker_x_unit, L_rocker_y_unit, R_rocker_x_unit, R_rocker_y_unit, L_pressure_unit, R_pressure_unit, B_fader_unit};
  adc_channel_t analog_input_channel[7] =
    {L_rocker_x_channel, L_rocker_y_channel, R_rocker_x_channel, R_rocker_y_channel, L_pressure_channel, R_pressure_channel, B_fader_channel};
  string analog_input_direction[7] =
    {L_rocker_x_direction, L_rocker_y_direction, R_rocker_x_direction, R_rocker_y_direction, L_pressure_direction, R_pressure_direction, B_fader_direction};
  adc_oneshot_unit_handle_t adc_handle[2];

  AnalogConfig LX = { .name = "LX",  .max = 2900,  .min = 900,   .middle = 1910, };
  AnalogConfig LY = { .name = "LY",  .max = 3100,  .min = 1500,  .middle = 2340, };
  AnalogConfig RX = { .name = "RX",  .max = 3200,  .min = 900,   .middle = 2110, };
  AnalogConfig RY = { .name = "RY",  .max = 3100,  .min = 900,   .middle = 2010, };
  AnalogConfig LP = { .name = "LP",  .max = 2950,  .min = 1050,  .middle = 1880, };
  AnalogConfig RP = { .name = "RP",  .max = 2950,  .min = 1050,  .middle = 1880, };
  AnalogConfig BF = { .name = "BF",  .max = 2950,  .min = 1050,  .middle = 1880, };

  void Scan() {
    
    for(uint8_t i = 0; i < 7; i++)
    {
      int result[3];
      for(uint8_t j = 0; j < 3; j++)
      {
        adc_oneshot_read((analog_input_unit[i] == ADC_UNIT_1 ? adc_handle[0] : adc_handle[1]), analog_input_channel[i], &result[j]);
      }
      if(analog_input_direction[i] == "Negative") 
        reading[i] = 0b0000111111111111 - middleOfThree(result[0], result[1], result[2]);
      else 
        reading[i] = middleOfThree(result[0], result[1], result[2]);
    }
    
    if (!dialPtr.empty())
      dialMode = true;
    else
    {
      dialMode = false;
      dial_callback = nullptr;
    }

    // MLOGD("LX", "LX = %d", GetRaw("LX"));
    // MLOGD("LY", "LY = %d", GetRaw("LY"));
    // MLOGD("RX", "RX = %d", GetRaw("RX"));
    // MLOGD("RY", "RY = %d", GetRaw("RY"));

    ModWheel();
    if(!dialMode) PitchWheel();
    if(!dialMode) DirectPad();
    if (dialMode) 
    {
      if(Device::KeyPad::Rocker()) 
      {
        DisableDial();
        Device::KeyPad::LRockerState.Clear();
        Device::KeyPad::RRockerState.Clear();
        return;
      }
      Dial();
    }

  }

  void PitchWheel()
  {
    int8_t lx = GetRocker(LX);
    int8_t ly = GetRocker(LY);
    int16_t change = ly * 64 + lx * 5.3;
    if (change > 0x1FFE) change = 0x1FFE;
    if (change < - 0x1FFE) change = - 0x1FFE;

    uint16_t pitch = 0x1FFF + change;
    // if (pitch != lastPitch){
    //   int8_t channel = MatrixOS::UserVar::global_channel;
    //   //MLOGD("Pitch Wheel", "lx:%d, ly:%d, pitch:%d, ch:%d", lx, ly, pitch, channel);
    //   lastPitch = pitch;
    //   MatrixOS::MIDI::Send(MidiPacket(0, PitchChange, channel, pitch));
    // }
  }

  void ModWheel()
  {
    if(!dialMode)
    {
      uint8_t mod = GetRaw("BF") / 32;
      if (lastMod != mod){
        int8_t channel = MatrixOS::UserVar::global_channel;
        // MLOGD("Mod Wheel", "mod:%d, ch:%d", mod, channel);
        lastMod = mod;
        MatrixOS::MIDI::Send(MidiPacket(0, ControlChange, channel, 1, mod));
      }
    }
  }

  inline uint16_t* GetPtr(string input)
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

  inline uint16_t GetRaw(string input)
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

  inline int8_t GetRocker(AnalogConfig config)
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

  void Init() 
  {
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
}