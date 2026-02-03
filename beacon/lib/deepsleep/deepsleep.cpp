#include "deepsleep.h"

namespace deepsleep {

static inline uint32_t wdtRrenMaskForChannel(uint8_t ch) {
  // NRF52: RREN tiene bits RR0..RR7
  // WDT_RREN_RR0_Msk está definido; usamos shift seguro.
  return (1UL << ch);
}

void startLfclk() {
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {}
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
}

bool wasResetByWdtAndClear() {
  const uint32_t mask = POWER_RESETREAS_DOG_Msk;
  bool was = (NRF_POWER->RESETREAS & mask) != 0;
  if (was) {
    // Limpia el flag escribiendo 1 en el bit correspondiente
    NRF_POWER->RESETREAS = mask;
  }
  return was;
}

void startWdt(const Config& cfg) {
  // CONFIG: bit SLEEP_Run controla si el WDT corre durante sleep
  NRF_WDT->CONFIG = ((cfg.runWdtInSleep ? WDT_CONFIG_SLEEP_Run : WDT_CONFIG_SLEEP_Pause)
                    << WDT_CONFIG_SLEEP_Pos);

  // CRV en ticks de LFCLK (32768 Hz)
  // timeoutSeconds * 32768
  const uint32_t crv = 32768UL * cfg.timeoutSeconds;
  NRF_WDT->CRV = crv;

  // Habilita RR channel
  NRF_WDT->RREN = wdtRrenMaskForChannel(cfg.rrChannel);

  // Nota: NO hacemos feed (RR[x]) porque tu estrategia es reset por timeout.
  NRF_WDT->TASKS_START = 1;
}

void disableUsbd() {
#if defined(NRF_USBD)
  NRF_USBD->ENABLE = 0;
  NRF_USBD->USBPULLUP = 0;
#endif
}

void blinkEntry(const Config& cfg) {
  if (cfg.ledPin < 0) return;

  pinMode(cfg.ledPin, OUTPUT);

  if (cfg.blinkMs == 0) return;

  // tu patrón: LOW -> delay -> HIGH
  digitalWrite(cfg.ledPin, LOW);
  delay(cfg.blinkMs);
  digitalWrite(cfg.ledPin, HIGH);
}

[[noreturn]] void sleepForeverWfe() {
  while (true) {
    __WFE();
    // Si necesitas evitar “despertadas” falsas por eventos,
    // puedes usar el patrón:
    // __SEV();
    // __WFE();
    // __WFE();
  }
}

} // namespace deepsleep
