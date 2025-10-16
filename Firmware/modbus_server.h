#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <EthernetENC.h>
#include <ModbusTCPSlave.h>
#include "netcfg.h"
// ========================
// Configuración del servidor Modbus TCP
// ========================

// Número máximo de Holding Registers
constexpr size_t HREG_COUNT = 2;

// Pines hardware
constexpr uint8_t ENC28J60_CS = D2; // ajustar según hardware

// Variables globales Modbus (accesibles externamente)
extern uint16_t hregs[HREG_COUNT];

// Inicialización de red y Modbus
bool netInit();      // configura Ethernet con datos persistidos
void modbusInit();   // inicializa mapa de registros
void printNetInfo(); // imprime diagnóstico de red
void modbusTask();   // atender peticiones Modbus
