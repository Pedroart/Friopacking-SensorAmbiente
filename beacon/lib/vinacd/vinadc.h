#pragma once
#include <Arduino.h>

#ifndef MIN_MV
    #define MIN_MV 3000
#endif

#ifndef MAX_MV
    #define MAX_MV 4200
#endif

#ifndef PIN_ADC
    #define PIN_ADC A0
#endif

#ifndef PIN_GND_SW
    #define PIN_GND_SW D0
#endif

#ifndef N_SAMPLES
    #define N_SAMPLES 10
#endif

#ifndef DIV_FACTOR
    #define DIV_FACTOR 2.0f
#endif

#ifndef VREF
    #define VREF 3.3f
#endif

void vin_adc_begin();
uint16_t vin_adc_read_raw_avg(int8_t samples = N_SAMPLES);
uint16_t vin_adc_raw_to_vin_mV(uint16_t raw);
uint8_t vin_mV_to_percent(uint16_t mv);