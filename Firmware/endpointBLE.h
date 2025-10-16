#pragma once
#include <Arduino.h>
#include <bluefruit.h>
#include "netcfg.h"
#include "temp117_utils.h"

// ==========================================================
// Estado del módulo BLE
// ==========================================================
extern bool bleEnabled;
extern BLEUart bleuart;

// ==========================================================
// Funciones públicas
// ==========================================================
void bleInit();               // Inicializa el stack BLE
void bleEnable(bool state);   // Activa o desactiva BLE en tiempo real
void bleLoop();               // Llamar periódicamente
void bleHandleCommand(const String &cmd);
void bleSend(const String &msg);
