#include "vinadc.h"

void vin_adc_begin() {
  analogReadResolution(12);
  pinMode(PIN_ADC, INPUT);
  pinMode(PIN_GND_SW, INPUT);
}

uint16_t vin_adc_read_raw_avg(int8_t samples) {
  pinMode(PIN_GND_SW, OUTPUT);
  digitalWrite(PIN_GND_SW, LOW);

  delay(5);
  (void)analogRead(PIN_ADC);
  delayMicroseconds(300);

  uint32_t acc = 0;
  for (uint8_t i = 0; i < samples; i++) {
    acc += analogRead(PIN_ADC);
    delayMicroseconds(300);
  }

  pinMode(PIN_GND_SW, INPUT);

  float rawAvg = (float)acc / (float)samples;
  return (uint16_t)(rawAvg + 0.5f);
}

uint16_t vin_adc_raw_to_vin_mV(uint16_t raw) {
  float v_adc = (raw * VREF) / 4095.0f;
  float vin   = v_adc * DIV_FACTOR;
  int mv = (int)(vin * 1000.0f + 0.5f);
  if (mv < 0) mv = 0;
  return (uint16_t)mv;
}

uint8_t vin_mV_to_percent(uint16_t mv) {

  if (mv <= MIN_MV) return 0;
  if (mv >= MAX_MV) return 100;

  return (uint8_t)((mv - MIN_MV) * 100 / (MAX_MV - MIN_MV));
}
