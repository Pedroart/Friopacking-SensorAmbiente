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
#define KEY_TX_POWER "tx_power_u8";

static const uint8_t KEY[16] = {
  0xA3, 0x7F, 0x1C, 0xD9, 0x88, 0x4E, 0x21, 0xB6,
  0x59, 0x02, 0xEF, 0xC4, 0x6A, 0x90, 0x13, 0xDD
};

