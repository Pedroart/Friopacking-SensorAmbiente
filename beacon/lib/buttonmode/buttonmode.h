#pragma once
#include <Arduino.h>

#ifndef BTN_PINN
  #define BTN_PINN D7   // cámbialo si tu placa usa otro
#endif

#ifndef BTN_ACTIVE_LOW
  #define BTN_ACTIVE_LOW 1      // la mayoría de botones van a GND
#endif

#ifndef BTN_USE_PULLUP
  #define BTN_USE_PULLUP 1
#endif

#ifndef BTN_DEBOUNCE_MS
  #define BTN_DEBOUNCE_MS 25
#endif

#ifndef BTN_SHORT_MAX_MS
  #define BTN_SHORT_MAX_MS 450
#endif

#ifndef BTN_LONG_MIN_MS
  #define BTN_LONG_MIN_MS 900
#endif

#ifndef BTN_VERY_LONG_MIN_MS
  #define BTN_VERY_LONG_MIN_MS 3000
#endif

namespace button_mode {

enum class PressType : uint8_t {
  None = 0,
  Short,
  Long,
  VeryLong
};

struct Config {
  int pin = BTN_PINN;
  bool activeLow = (BTN_ACTIVE_LOW != 0);
  bool usePullup = (BTN_USE_PULLUP != 0);

  uint16_t debounceMs = BTN_DEBOUNCE_MS;
  uint16_t shortMaxMs = BTN_SHORT_MAX_MS;
  uint16_t longMinMs  = BTN_LONG_MIN_MS;
  uint16_t veryLongMinMs = BTN_VERY_LONG_MIN_MS;
};

// Inicializa el pin
void begin(const Config& cfg);

// Detecta pulsación que ocurra DURANTE una ventana al arranque.
// - espera hasta windowMs para ver si el botón se presiona
// - si se presiona, mide cuánto dura (hasta maxHoldMs)
// Devuelve tipo: Short/Long/VeryLong/None
PressType detectAtBoot(const Config& cfg,
                       uint32_t windowMs = 1200,
                       uint32_t maxHoldMs = 6000);

// Helpers
bool isPressedNow(const Config& cfg);

} // namespace button_mode
