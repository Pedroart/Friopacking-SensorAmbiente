#include "deepsleep.h"

namespace deepsleep {

static inline uint32_t wdtRrenMaskForChannel(uint8_t ch) {
  // NRF52: RREN tiene bits RR0..RR7
  // WDT_RREN_RR0_Msk está definido; usamos shift seguro.
  return (1UL << ch);
}

void startLfclk() {
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos);
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
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
 
  NRF_RTC2->TASKS_STOP = 1;
  NRF_RTC2->TASKS_CLEAR = 1;

  // OJO: con PRESCALER=0 el tick es 32768 Hz exacto (más simple)
  NRF_RTC2->PRESCALER = 0;

  // Compare en ticks
  NRF_RTC2->CC[0] = cfg.timeoutSeconds * 32768UL;

  NRF_RTC2->EVENTS_COMPARE[0] = 0;
  NRF_RTC2->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;
  NRF_RTC2->INTENSET = RTC_INTENSET_COMPARE0_Msk;

  NVIC_ClearPendingIRQ(RTC2_IRQn);
  NVIC_EnableIRQ(RTC2_IRQn);

  NRF_RTC2->TASKS_START = 1;
}

void disableUsbd() {
#if defined(NRF_USBD)
  NRF_USBD->ENABLE = 0;
  NRF_USBD->USBPULLUP = 0;
#endif
}

extern "C" void RTC2_IRQHandler(void)
{
  if (NRF_RTC2->EVENTS_COMPARE[0]) {
    NRF_RTC2->EVENTS_COMPARE[0] = 0;
    NVIC_SystemReset(); // <<--- tu “wake”: realmente es reboot
  }
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
  // Limpia evento latente y duerme
  __SEV();
  __WFE();
  __WFE();

  while (true) {
    __WFE();   // o __WFI();
  }
}

} // namespace deepsleep

