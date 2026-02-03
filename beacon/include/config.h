#pragma once
#include <Arduino.h>

// ===================== TMP117 =====================
#define SDA_PIN D4
#define SCL_PIN D5
static const uint8_t TMP117_ADDR  = 0x49;      // ADDR a V+
static const uint8_t REG_TEMP     = 0x00;
static const float   TMP117_LSB_C = 0.0078125f;

// ===================== VIN ADC (tu hardware) =====================
// Punto medio divisor -> A3 (P0.29)
// R2 a "tierra conmutada" -> D6 (P1.11)
#define PIN_ADC    A3
#define PIN_GND_SW D6
static const float VREF = 3.3f;
static const float DIV_FACTOR = 2.0f;          // 1M/1M
static const uint8_t N_SAMPLES = 8;

// ===================== Protocolo ADV =====================
static const uint8_t  PROTO_VER = 0x01;
static const uint16_t DEV_ID    = 0x0002;      // <<< CAMBIA por nodo

// Clave de grupo AES-128 (16 bytes) - MISMA en gateway
static const uint8_t GROUP_KEY[16] = {
  0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
  0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10
};

// Manufacturer payload (23 bytes)
static const size_t MFG_LEN = 23;

// Persistencia boot_id
static const char *BOOT_FILE = "/boot.bin";
