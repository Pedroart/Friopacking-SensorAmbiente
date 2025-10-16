#include "endpointBLE.h"

// =====================================================
// === Estado BLE y buffers
// =====================================================
bool bleEnabled = false;
static char cmdBuffer[64];
static uint8_t idx = 0;

// =====================================================
// === Servicio GATT personalizado
// =====================================================
BLEService      serviceCmd(0xFFF0);
BLECharacteristic charRX(0xFFF1);
BLECharacteristic charTX(0xFFF2);

void rxCallback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len);

// =====================================================
// === Inicialización general BLE (stack Bluefruit)
// =====================================================
void bleInit() {
  Bluefruit.begin();
  Bluefruit.setName("TMP117-BLE");

  // Definimos el servicio y características
  serviceCmd.begin();

  charRX.setProperties(CHR_PROPS_WRITE | CHR_PROPS_WRITE_WO_RESP);
  charRX.setPermission(SECMODE_OPEN, SECMODE_OPEN);
  charRX.setWriteCallback(rxCallback);
  charRX.setMaxLen(sizeof(cmdBuffer));
  charRX.begin();

  charTX.setProperties(CHR_PROPS_NOTIFY);
  charTX.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  charTX.setMaxLen(64);
  charTX.begin();

  Bluefruit.Advertising.addService(serviceCmd);
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244); // rápido y visible
  Bluefruit.Advertising.setFastTimeout(30);   // 30 seg en modo rápido


  bleEnable(false);  // arranca desactivado
  Serial.println(F("[BLE] Inicializado (inactivo)"));
}

// =====================================================
// === Habilitar o deshabilitar BLE dinámicamente
// =====================================================
void bleEnable(bool state) {
  if (state && !bleEnabled) {
    bleEnabled = true;
    Bluefruit.Advertising.addName();
    Bluefruit.Advertising.start(0);  // 0 = infinito
    Serial.println(F("[BLE] Activado"));
    bleSend("OK BLE=ON\n");
  } 
  else if (!state && bleEnabled) {
    bleEnabled = false;
    Bluefruit.Advertising.stop();
    Bluefruit.disconnect(0xFFFF);
    Serial.println(F("[BLE] Desactivado"));
  }
}

// =====================================================
// === Loop principal (nada especial, BLE maneja eventos)
// =====================================================
void bleLoop() {
  // No es necesario hacer polling, todo se maneja en callback
}

// =====================================================
// === Callback de escritura (cuando llega un comando)
// =====================================================
void rxCallback(uint16_t conn_hdl, BLECharacteristic* chr, uint8_t* data, uint16_t len) {
  if (len >= sizeof(cmdBuffer)) len = sizeof(cmdBuffer) - 1;
  memcpy(cmdBuffer, data, len);
  cmdBuffer[len] = '\0';
  bleHandleCommand(String(cmdBuffer));
}

// =====================================================
// === Procesador de comandos BLE
// =====================================================
void bleHandleCommand(const String &cmd) {
  Serial.print(F("[BLE CMD] ")); Serial.println(cmd);

  if (cmd.equalsIgnoreCase("BLE=ON")) {
    bleEnable(true);
    bleSend("OK BLE=ON\n");
  }
  else if (cmd.equalsIgnoreCase("BLE=OFF")) {
    bleSend("OK BLE=OFF\n");
    bleEnable(false);
  }
  else if (cmd.startsWith("IP=")) {
    IPAddress newIP;
    if (parseIPv4(cmd.substring(3), newIP)) {
      setIP(newIP);
      bleSend("OK IP SAVED: " + cmd.substring(3) + "\n");
    } else {
      bleSend("ERR BAD IP FORMAT\n");
    }
  }
  else if (cmd.startsWith("GW=")) {
    IPAddress newGW;
    if (parseIPv4(cmd.substring(3), newGW)) {
      setGateway(newGW);
      bleSend("OK GW SAVED: " + cmd.substring(3) + "\n");
    } else {
      bleSend("ERR BAD GW FORMAT\n");
    }
  }
  else if (cmd.startsWith("MASK=")) {
    IPAddress newMask;
    if (parseIPv4(cmd.substring(5), newMask)) {
      setMask(newMask);
      bleSend("OK MASK SAVED: " + cmd.substring(5) + "\n");
    } else {
      bleSend("ERR BAD MASK FORMAT\n");
    }
  } 
  else if (cmd.startsWith("AVG=")) {
    int val = cmd.substring(4).toInt();
    tmp117.setAveragedSampleCount((tmp117_average_count_t)val);
    bleSend("OK AVG=" + String(val) + "\n");
  } 
  else if (cmd.startsWith("CAL=")) {
    float sp = cmd.substring(4).toFloat();
    if (tempCalibrate(sp)) bleSend("OK CAL\n");
    else bleSend("ERR CAL\n");
  } 
  else if (cmd.equalsIgnoreCase("TEMP?")) {
    bleSend("TEMP=" + String(readTempC(), 2) + "\n");
  }
  else if (cmd.equalsIgnoreCase("RESET")) {
    bleSend("REBOOT...\n");
    delay(200);
    NVIC_SystemReset();
  }
  else {
    bleSend("ERR UNKNOWN\n");
  }
}

// =====================================================
// === Enviar datos al cliente BLE (TX notify)
// =====================================================
void bleSend(const String &msg) {
  charTX.notify(msg.c_str(), msg.length());
  Serial.print(F("[BLE TX] ")); Serial.print(msg);
}
