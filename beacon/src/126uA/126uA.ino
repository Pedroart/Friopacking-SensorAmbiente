#include <Arduino.h>
#include "nrf.h"

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  // ¿Reset por WDT?
  if (NRF_POWER->RESETREAS & POWER_RESETREAS_DOG_Msk) {
    NRF_POWER->RESETREAS = POWER_RESETREAS_DOG_Msk; // limpia flag
    // Aquí vienes del "wake"
  }

  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {}
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;

  // Configura Watchdog
  NRF_WDT->CONFIG = (WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos); // corre en sleep
  NRF_WDT->CRV = 32768 * 16;   // ~16 segundos (LFCLK)
  NRF_WDT->RREN = WDT_RREN_RR0_Msk;
  NRF_WDT->TASKS_START = 1;

  // Acción rápida
  digitalWrite(LED_BUILTIN, LOW);
  delay(30);
  digitalWrite(LED_BUILTIN, HIGH);

/* Apagar el USB no hizo algun cambio*/
  NRF_USBD->ENABLE = 0;
  NRF_USBD->USBPULLUP = 0;

  // Dormir → WDT reseteará
  while (1) {
    __WFE();

  /* Subio 0.003mA
  __SEV();
  __WFE();
  */
  }
}

void loop() {
  // Nunca llega aquí
}
