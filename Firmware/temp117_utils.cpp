#include "temp117_utils.h"

Adafruit_TMP117 tmp117;

bool tempSetOffsetC(float offsetC) {
  if (!tmp117.setOffset(offsetC)) {
    Serial.println(F("[TMP117] ERROR setOffset"));
    return false;
  }
  return true;
}

bool tempCalibrate(float setpoint ) {
  
  if (tempSetOffsetC(0)){ return false;}
  if (tmp117.getOffset() < 0.01) { return false;}

  float error = setpoint - readTempC();
  return tempSetOffsetC(error);
}

float tempGetOffsetC() {
  return tmp117.getOffset();
}


float readTempC() {
  sensors_event_t e;
  if (!tmp117.getEvent(&e)) return 255;
  return e.temperature;
}

bool sensorSetup(
  tmp117_average_count_t avg,
  tmp117_delay_t         delay,
  tmp117_mode_t          mode,
  uint8_t                i2c_addr,
  uint8_t                sda,
  uint8_t                scl
) {
  // En varios cores conviene fijar pines antes del begin()
  Wire.setPins(sda, scl);
  Wire.begin();

  if (!tmp117.begin(i2c_addr, &Wire)) {
    Serial.println(F("[TMP117] No detectado"));
    return false;
  }

  // Configura averaging
  if (!tmp117.setAveragedSampleCount(avg)) {
    Serial.println(F("[TMP117] ERROR setAveragedSampleCount"));
    return false;
  }

  // Configura retardo mínimo entre lecturas nuevas
  if (!tmp117.setReadDelay(delay)) {
    Serial.println(F("[TMP117] ERROR setReadDelay"));
    return false;
  }

  // Configura modo de operación
  if (!tmp117.setMeasurementMode(mode)) {
    Serial.println(F("[TMP117] ERROR setMeasurementMode"));
    return false;
  }

  Serial.println(F("[TMP117] OK (avg/delay/mode configurados)"));
  return true;
}
