#include "tmp117.h"
#include <Wire.h>

static bool i2cRead16(uint8_t addr, uint8_t reg, uint16_t &val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom((int)addr, 2) != 2) return false;
  val = ((uint16_t)Wire.read() << 8) | Wire.read();
  return true;
}

bool tmp117_begin() {
  Wire.setPins(SDA_PIN, SCL_PIN);
  Wire.begin();
  return true;
}

bool tmp117_read_x100(int16_t &temp_x100) {
  uint16_t raw;
  if (!i2cRead16(TMP117_ADDR, REG_TEMP, raw)) return false;

  float tC = ((int16_t)raw) * TMP117_LSB_C;
  temp_x100 = (int16_t)(tC * 100.0f + (tC >= 0 ? 0.5f : -0.5f));
  return true;
}
