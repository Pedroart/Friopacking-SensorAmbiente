#include "modbus_server.h"

// ---------- CONFIG RED ----------
static byte mac[] = { 0xDE, 0xAD, 0xBE, 0x52, 0x84, 0x00 };

// ---------- SERVIDORES ----------
uint16_t hregs[HREG_COUNT] = {0};
EthernetServer server(502);
ModbusTCPSlave modbus;

// =============================================================
// Inicializa la red Ethernet usando datos persistidos en memoria
// =============================================================
bool netInit() {
  SPI.begin();
  Ethernet.init(ENC28J60_CS);

  IPAddress ip   = getIP();
  IPAddress gw   = getGW();
  IPAddress mask = getMask();

  Serial.println(F("[NetInit] Configurando Ethernet..."));
  Serial.printf("   IP   : %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
  Serial.printf("   GW   : %u.%u.%u.%u\n", gw[0], gw[1], gw[2], gw[3]);
  Serial.printf("   MASK : %u.%u.%u.%u\n", mask[0], mask[1], mask[2],mask[3]);

  Ethernet.begin(mac, ip, gw, gw, mask);
  delay(300);

  IPAddress myIP = Ethernet.localIP();
  Serial.printf("[NetInit] IP activa: %u.%u.%u.%u\n", myIP[0], myIP[1], myIP[2], myIP[3]);
  delay(300);
  server.begin();
  Serial.println(F("[NetInit] Servidor Modbus TCP iniciado."));
  return true;
}

// =============================================================
// Inicializa el espacio de registros Modbus
// =============================================================
void modbusInit() {
  modbus.configureHoldingRegisters(hregs, HREG_COUNT);

  // valores iniciales (por ejemplo, temperatura x10)
  hregs[0] = 0;
  hregs[1] = 0;
}

// =============================================================
// Atiende el tráfico Modbus TCP (llamar en loop())
// =============================================================
void modbusTask() {
  EthernetClient client = server.available();
  if (client) {
    modbus.poll(client);
  }
}

// =============================================================
// Muestra info de red actual
// =============================================================
void printNetInfo() {
  Serial.print(F("IP: "));   Serial.println(Ethernet.localIP());
  Serial.print(F("GW: "));   Serial.println(Ethernet.gatewayIP());
  Serial.print(F("MASK: ")); Serial.println(Ethernet.subnetMask());

#if defined(ETHERNET_H) || defined(ethernet_h_) || defined(_ETHERNETENC_H_)
  auto ls = Ethernet.linkStatus();
  Serial.print(F("LINK: "));
  if (ls == LinkON)       Serial.println(F("ON"));
  else if (ls == LinkOFF) Serial.println(F("OFF"));
  else                    Serial.println(F("UNKNOWN"));
#endif
}
