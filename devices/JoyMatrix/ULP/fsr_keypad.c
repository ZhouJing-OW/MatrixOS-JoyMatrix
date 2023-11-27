#include <stdint.h>
#include "ulp_riscv_utils.h"
#include "ulp_riscv_gpio.h"
#include "ulp_riscv_adc_ulp_core.h"

#define X_SIZE 16
#define Y_SIZE 4
#define SAMPLES 3

volatile gpio_num_t keypad_write_pins[4] = 
{
  GPIO_NUM_4,
  GPIO_NUM_5,
  GPIO_NUM_6,
  GPIO_NUM_7,
};

volatile adc_channel_t keypad_read_adc_channel[10] = 
{
  ADC_CHANNEL_0,
  ADC_CHANNEL_1,
  ADC_CHANNEL_3,
  ADC_CHANNEL_4,
  ADC_CHANNEL_5,
  ADC_CHANNEL_6,
  ADC_CHANNEL_7,
  ADC_CHANNEL_2,
  ADC_CHANNEL_8,
  ADC_CHANNEL_9
};

volatile uint16_t result[X_SIZE][Y_SIZE][SAMPLES];

volatile uint32_t count;
volatile uint32_t latest;

int main(void)
{
  count = 0;
  for (uint8_t x = 0; x < 4; x++)
  {
    ulp_riscv_gpio_init(keypad_write_pins[x]);
    ulp_riscv_gpio_output_enable(keypad_write_pins[x]);
    ulp_riscv_gpio_set_output_mode(keypad_write_pins[x], RTCIO_MODE_OUTPUT);
    ulp_riscv_gpio_output_level(keypad_write_pins[x], 0);
  }
  while(true)
  {
    uint8_t sample_index = count % SAMPLES;
    for (uint8_t x = 0; x < 16; x++)
    {
      for(uint8_t a = 0; a < 4; a++) {
        ulp_riscv_gpio_output_level(keypad_write_pins[a], (x >> a) & 1);
      }

      for (uint8_t y = 0; y < 10; y++)
      {
        result[x + (8 * (y % 2 ))][y / 2][sample_index] = ulp_riscv_adc_read_channel(ADC_UNIT_1, keypad_read_adc_channel[y]);// 8 x 10 to 16 x 5
      }

      ulp_riscv_gpio_output_level(keypad_write_pins[x], 0);
    }

    for(uint8_t a = 0; a < 4; a++) {
        ulp_riscv_gpio_output_level(keypad_write_pins[a], 0);
      }
    latest = sample_index;
    count++;
  }
}