// temp117_utils.h
#pragma once
#include <Arduino.h>
#include <Adafruit_TMP117.h>

// ---- Defaults de pines I2C (puedes cambiarlos aquí) ----
static const uint8_t SDA_DEFAULT  = 5;
static const uint8_t SCL_DEFAULT  = 4;

// Instancia global del sensor (definirla una sola vez en tu .ino/.cpp)
// En el .ino:  Adafruit_TMP117 tmp117;
extern Adafruit_TMP117 tmp117;

// ----------------- API -----------------


bool sensorSetup(
  tmp117_average_count_t avg   = TMP117_AVERAGE_8X,
  tmp117_delay_t         delay = TMP117_DELAY_0_MS,
  tmp117_mode_t          mode  = TMP117_MODE_CONTINUOUS,
  uint8_t                i2c_addr = 0x49,
  uint8_t                sda = SDA_DEFAULT,
  uint8_t                scl = SCL_DEFAULT
);

bool tempSetOffsetC(float offsetC);

bool tempCalibrate(float setpoint);

float tempGetOffsetC();

float readTempC();
