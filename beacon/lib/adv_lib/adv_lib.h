#pragma once
#include <Arduino.h>

/* ===================== Config por defecto (redefinible) ===================== */

// Nombre BLE por defecto
#ifndef ADV_DEFAULT_NAME
  #define ADV_DEFAULT_NAME "XIAO-TLM"
#endif

// Intervalo ADV en unidades BLE (0.625 ms). 160 = 100 ms
#ifndef ADV_INTERVAL_MIN_UNITS
  #define ADV_INTERVAL_MIN_UNITS 160
#endif

#ifndef ADV_INTERVAL_MAX_UNITS
  #define ADV_INTERVAL_MAX_UNITS 160
#endif

// Duración del advertising al hacer start(seconds)
#ifndef ADV_START_SECONDS
  #define ADV_START_SECONDS 1
#endif

// Siempre agregamos flags
#ifndef ADV_ALWAYS_FLAGS
  #define ADV_ALWAYS_FLAGS 1
#endif

// Siempre agregamos TxPower (según tu requerimiento)
#ifndef ADV_ALWAYS_TXPOWER
  #define ADV_ALWAYS_TXPOWER 1
#endif

// Límite legacy ADV payload
#ifndef ADV_LEGACY_MAX_BYTES
  #define ADV_LEGACY_MAX_BYTES 31
#endif

/* ===================== API ===================== */

// Inicializa Bluefruit y configura nombre + intervalo.
void adv_begin(const char* name = ADV_DEFAULT_NAME);

// Cambia el nombre BLE (opcional)
void adv_set_name(const char* name);

// Cambia el intervalo (unidades BLE). Llamar después de adv_begin()
void adv_set_interval_units(uint16_t min_units, uint16_t max_units);

// Limpia ADV + ScanResponse (aunque no usemos SR, lo limpiamos)
void adv_clear();

// Detiene advertising
void adv_stop();

// Inicia advertising por X segundos (por defecto ADV_START_SECONDS)
void adv_start(uint16_t seconds = ADV_START_SECONDS);

// Publica Manufacturer Data en ADV legacy (31 bytes), sin Scan Response.
// Devuelve false si NO entra (con flags+txpower siempre).
bool adv_publish_mfg(const uint8_t* data, size_t len, uint16_t seconds = ADV_START_SECONDS);

// Utilidades de tamaño
size_t adv_overhead_bytes(); // bytes fijos usados por flags + txpower + MSD overhead
int    adv_max_mfg_payload(); // máximo payload permitido para data (g_mfg) con tu configuración
bool   adv_will_fit_mfg(size_t payload_len);

void adv_lib_end();