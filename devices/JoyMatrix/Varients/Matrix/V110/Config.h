namespace V110
{
  const gpio_num_t FN_Pin = GPIO_NUM_45;

  const gpio_num_t LED_Pin = GPIO_NUM_35;

  const gpio_num_t Key1_Pin = GPIO_NUM_4;
  const gpio_num_t Key2_Pin = GPIO_NUM_5;
  const gpio_num_t Key3_Pin = GPIO_NUM_6;
  const gpio_num_t Key4_Pin = GPIO_NUM_7;

  const gpio_num_t KeyRead1_Pin = GPIO_NUM_15;
  const gpio_num_t KeyRead2_Pin = GPIO_NUM_16;
  const gpio_num_t KeyRead3_Pin = GPIO_NUM_17;
  const gpio_num_t KeyRead4_Pin = GPIO_NUM_18;
  const gpio_num_t KeyRead5_Pin = GPIO_NUM_8;
  const gpio_num_t KeyRead6_Pin = GPIO_NUM_3;
  const gpio_num_t KeyRead7_Pin = GPIO_NUM_9;
  const gpio_num_t KeyRead8_Pin = GPIO_NUM_10;
  
  const gpio_num_t KeyRead9_Pin = GPIO_NUM_47;
  const gpio_num_t KeyRead10_Pin = GPIO_NUM_48;

  const gpio_num_t LRockerBtn_Pin = GPIO_NUM_21;
  const gpio_num_t RRockerBtn_Pin = GPIO_NUM_46;

  const adc_channel_t KeyRead1_ADC_CHANNEL = ADC_CHANNEL_4;
  const adc_channel_t KeyRead2_ADC_CHANNEL = ADC_CHANNEL_5;
  const adc_channel_t KeyRead3_ADC_CHANNEL = ADC_CHANNEL_6;
  const adc_channel_t KeyRead4_ADC_CHANNEL = ADC_CHANNEL_7;

  // const gpio_num_t TouchData_Pin = GPIO_NUM_34;
  // const gpio_num_t TouchClock_Pin = GPIO_NUM_33;

  // const gpio_num_t PowerCord_Pin = GPIO_NUM_47;

  // const gpio_num_t PMIC_INT_Pin = GPIO_NUM_45;


}

void Device::LoadV110() {
  ESP_LOGI("Device Init", "Matrix Pro V110 Config Loaded");
  led_pin = V110::LED_Pin;

  KeyPad::fn_pin = V110::FN_Pin;

  gpio_num_t _keypad_write_pins[4] = {
      V110::Key1_Pin, V110::Key2_Pin, V110::Key3_Pin, V110::Key4_Pin,
  };
  memcpy(KeyPad::keypad_write_pins, _keypad_write_pins, sizeof(_keypad_write_pins));

  gpio_num_t _keypad_read_pins[5] = {
      V110::KeyRead1_Pin, V110::KeyRead2_Pin, V110::KeyRead3_Pin, V110::KeyRead4_Pin, V110::KeyRead9_Pin,
  };
  memcpy(KeyPad::keypad_read_pins, _keypad_read_pins, sizeof(_keypad_read_pins));

  gpio_num_t _keypad_funcRead_pins[1] = {
      V110::KeyRead10_Pin,  
  };
  memcpy(KeyPad::keypad_funcRead_pins, _keypad_funcRead_pins, sizeof(_keypad_funcRead_pins));

  gpio_num_t _rockerBtn_pins[2] = {
      V110::LRockerBtn_Pin, V110::RRockerBtn_Pin,
  };
  memcpy(KeyPad::rockerBtn_pins, _rockerBtn_pins, sizeof(_rockerBtn_pins));

  adc_channel_t _keypad_read_adc_channel[4] = {
      V110::KeyRead1_ADC_CHANNEL, V110::KeyRead2_ADC_CHANNEL, V110::KeyRead3_ADC_CHANNEL, V110::KeyRead4_ADC_CHANNEL,
  };
  memcpy(KeyPad::keypad_read_adc_channel, _keypad_read_adc_channel, sizeof(_keypad_read_adc_channel));

  // KeyPad::touchData_Pin = V110::TouchData_Pin;
  // KeyPad::touchClock_Pin = V110::TouchClock_Pin;
  // uint8_t _touchbar_map[16] = {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3};
  // memcpy(KeyPad::touchbar_map, _touchbar_map, sizeof(_touchbar_map) * sizeof(_touchbar_map[0]));
};