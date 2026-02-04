#pragma once
#include <Arduino.h>
#include <Adafruit_TinyUSB.h> 
/* 
 Configuraciones para el módulo de deepsleep.
*/

#define DEEPSLEEP_WDT_TIMEOUT_S 30
#define DEEPSLEEP_DISABLE_USBD 1
#define DEEPSLEEP_BLINK_MS 30

#define BTN_PINN D7

#define PIN_ADC A3
#define PIN_GND_SW D6

#define KEY_DEVICE_ID "device_id"
#define KEY_CHIP_TEMP_OFFSET_X100 "chip_toff_x100"
