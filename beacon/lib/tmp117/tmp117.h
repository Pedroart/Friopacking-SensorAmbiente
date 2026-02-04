#pragma once

#include <Arduino.h>

/* ===================== Pines I2C ===================== */
#ifndef SDA_PIN
  #define SDA_PIN D4
#endif

#ifndef SCL_PIN
  #define SCL_PIN D5
#endif

/* ===================== TMP117 ===================== */
#ifndef TMP117_ADDR
  #define TMP117_ADDR 0x49   // ADDR a V+
#endif

#ifndef REG_TEMP
  #define REG_TEMP 0x00
#endif

#ifndef TMP117_LSB_C
  #define TMP117_LSB_C 0.0078125f
#endif

/* ===================== API ===================== */
bool tmp117_begin();
bool tmp117_read_x100(int16_t &temp_x100);


