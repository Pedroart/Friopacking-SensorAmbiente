#pragma once

#include <Arduino.h>
#include "nrf.h"

// ==============================
// Configurables (puedes redefinirlos antes del include)
// ==============================

// 1 = WDT corre en sleep (recomendado), 0 = se detiene en sleep
#ifndef DEEPSLEEP_WDT_RUN_IN_SLEEP
  #define DEEPSLEEP_WDT_RUN_IN_SLEEP 1
#endif

// Timeout del WDT en segundos (usa LFCLK 32768 Hz)
#ifndef DEEPSLEEP_WDT_TIMEOUT_S
  #define DEEPSLEEP_WDT_TIMEOUT_S 16
#endif

// Canal RR usado (0..7). Por defecto RR0.
#ifndef DEEPSLEEP_WDT_RR_CHANNEL
  #define DEEPSLEEP_WDT_RR_CHANNEL 0
#endif

// Parpadeo rápido al entrar (ms). 0 desactiva el blink
#ifndef DEEPSLEEP_BLINK_MS
  #define DEEPSLEEP_BLINK_MS 30
#endif

// Apagar USBD (algunas placas no cambian consumo, pero lo dejamos opcional)
#ifndef DEEPSLEEP_DISABLE_USBD
  #define DEEPSLEEP_DISABLE_USBD 1
#endif

// Pin LED (por defecto LED_BUILTIN si existe)
#ifndef DEEPSLEEP_LED_PIN
  #define DEEPSLEEP_LED_PIN LED_BUILTIN
#endif

namespace deepsleep {

struct Config {
  bool runWdtInSleep = (DEEPSLEEP_WDT_RUN_IN_SLEEP != 0);
  uint32_t timeoutSeconds = DEEPSLEEP_WDT_TIMEOUT_S;
  uint8_t rrChannel = DEEPSLEEP_WDT_RR_CHANNEL;
  uint32_t blinkMs = DEEPSLEEP_BLINK_MS;
  bool disableUsbd = (DEEPSLEEP_DISABLE_USBD != 0);
  int ledPin = DEEPSLEEP_LED_PIN;
};

// Inicia LFCLK (necesario para WDT con timeout por LFCLK)
void startLfclk();

// Devuelve true si el último reset fue por WDT y limpia el flag
bool wasResetByWdtAndClear();

// Configura e inicia WDT
void startWdt(const Config& cfg);

// Apaga USBD (si está habilitado)
void disableUsbd();

// Hace el blink rápido (si blinkMs > 0)
void blinkEntry(const Config& cfg);

// Entra a “deep sleep” por WFE. El WDT te va a resetear al expirar.
[[noreturn]] void sleepForeverWfe();

} // namespace deepsleep
