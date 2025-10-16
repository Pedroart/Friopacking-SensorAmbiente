#pragma once
#include <Arduino.h>
#include <IPAddress.h>

// =============================================================
// NetConfig – Configuración persistente de red (InternalFS)
// Guarda/lee IP, Gateway y Mask en /net.cfg
// =============================================================

// Inicialización del sistema de configuración
bool beginMemori();

// Setters (actualizan RAM y guardan a FS)
bool setIP(const IPAddress& ip);
bool setGateway(const IPAddress& gw);
bool setMask(const IPAddress& mask);
bool setAll(const IPAddress& ip, const IPAddress& gw, const IPAddress& mask);

// Getters (valores actuales en RAM)
IPAddress getIP();
IPAddress getGW();
IPAddress getMask();

// Restablecer valores por defecto
bool resetToDefaults();
bool parseIPv4(const String& s, IPAddress& out);