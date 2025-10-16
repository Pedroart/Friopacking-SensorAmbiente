#include <EthernetENC.h>    
#include <ModbusTCPSlave.h>
#include "temp117_utils.h"
#include "endpointBLE.h"
#include "modbus_server.h"
#include "netcfg.h"

static const uint32_t BLE_TOGGLE_HOLD_MS = 1500; // mantener 1.5s


void setup() {

  Serial.begin(115200);
  while (!Serial) {}

  if (beginMemori()) {
    Serial.println(F("[NetCfg] OK (cargado o creado)"));
  } else {
    Serial.println(F("[NetCfg] ERROR al inicializar"));
  }
  
  netInit();

  sensorSetup();
  bleInit();    
  
  modbusInit();
  buttonInit();
}


void loop() {
  buttonTask();
  bleLoop();
  modbusTask();
  if (buttonReleasedEvent()) {
    uint32_t dur = buttonLastPressDuration();
    if (dur > BLE_TOGGLE_HOLD_MS) {
      bleEnable(!bleEnabled);
      Serial.printf("[BUTTON] BLE %s\n", bleEnabled ? "ON" : "OFF");
    }
  }

  float temperatura = readTempC();
  hregs[1] = (int16_t)(temperatura * 10.0f);   // escala x10
  //Serial.printf("[LOOP]Temp: %.1f°C  →  hregs[1]=%d\n", temperatura, hregs[1]);

}
