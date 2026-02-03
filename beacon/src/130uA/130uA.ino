#include <Arduino.h>
#include "nrf.h"

static volatile bool woke = false;

// Interrupción del RTC2 (NO choca con FreeRTOS)
extern "C" void RTC2_IRQHandler(void) {
  if (NRF_RTC2->EVENTS_COMPARE[0]) {
    NRF_RTC2->EVENTS_COMPARE[0] = 0;
    woke = true;
    NRF_RTC2->TASKS_CLEAR = 1; // reinicia contador
  }
}

static void lfclk_start() {
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {}
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
}

static void rtc2_setup_wakeup_10s() {
  lfclk_start();

  // Stop + clear
  NRF_RTC2->TASKS_STOP  = 1;
  NRF_RTC2->TASKS_CLEAR = 1;

  // Tick = 100 Hz -> prescaler 327 (32768/(327+1)=100)
  NRF_RTC2->PRESCALER = 327;

  // 10s = 1000 ticks
  NRF_RTC2->CC[0] = 1000;

  // Enable compare0 event + interrupt
  NRF_RTC2->EVTENSET = RTC_EVTENSET_COMPARE0_Msk;
  NRF_RTC2->INTENSET = RTC_INTENSET_COMPARE0_Msk;

  NVIC_ClearPendingIRQ(RTC2_IRQn);
  NVIC_SetPriority(RTC2_IRQn, 6);
  NVIC_EnableIRQ(RTC2_IRQn);

  NRF_RTC2->TASKS_START = 1;
}

static void sleep_until_rtc() {
  woke = false;
  while (!woke) {
    __WFE();
    __SEV();
    __WFE();
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  rtc2_setup_wakeup_10s();

  // duerme hasta el primer wake
  sleep_until_rtc();
}

void loop() {
  // se ejecuta cada 10 segundos
  digitalWrite(LED_BUILTIN, LOW);
  delay(30);
  digitalWrite(LED_BUILTIN, HIGH);

  sleep_until_rtc();
}
