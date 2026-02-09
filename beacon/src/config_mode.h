#pragma once
#include <Arduino.h>
#include <stdint.h>

namespace config_mode {

// Tiempo sin actividad para auto-apagado (ms)
constexpr uint32_t DEFAULT_IDLE_TIMEOUT_MS = 120 * 1000; // 60s

struct Options {
  uint32_t idle_timeout_ms = DEFAULT_IDLE_TIMEOUT_MS; // apagado por inactividad
  uint32_t blink_period_ms = 300;                     // parpadeo
};

struct Values {
  uint8_t device_id = 0;  // 0..50
  uint8_t tx_power  = 0;  // 0..50 (escala tuya)
};

// Inicializa BLE + servicio config (no bloquea)
void begin(const Options& opt = Options{});

// Llama en loop (parpadeo, timeout, etc). Retorna true si debe apagarse.
bool tick();

// Últimos valores guardados (lo que está en KV)
Values load();

// Para depurar
uint32_t lastActivityMs();

} // namespace config_mode
