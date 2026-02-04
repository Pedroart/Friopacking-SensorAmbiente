
#include <Arduino.h>
#include <Adafruit_SPIFlash.h>
#include <nrf.h>
#include <nrf_gpio.h>
#include <nrf_power.h>
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
    // __WFE();
    // Si necesitas evitar “despertadas” falsas por eventos,
    // puedes usar el patrón:
    //__SEV();
    //__WFE();
    //__WFE();
    __WFI();
  }
}


// Si usas Bluefruit/SoftDevice


static Adafruit_FlashTransport_QSPI g_flashTransport;

// ---------- GPIO policy: elige UNA ----------
// 1) Recomendado si el pin realmente NO está conectado:
static inline void gpio_unused_to_input_nopull(uint32_t pin) {
  nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL);
}

// 2) Igual que tu código (puede consumir si algo fuerza el pin a GND):
static inline void gpio_unused_to_input_pullup(uint32_t pin) {
  nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);
}

// Ajusta aquí tu política:
static inline void gpio_unused_cfg(uint32_t pin) {
  gpio_unused_to_input_nopull(pin);   // <-- recomendado
  //gpio_unused_to_input_pullup(pin); // <-- alternativo
}

// Poner todos los GPIOs no usados a un estado "seguro"
static void disableGPIOs_range(uint32_t start, uint32_t end)
{
  for (uint32_t i = start; i <= end; i++) {
    // Excepciones (ajústalas a tu hardware)
    if (i == 0 || i == 1 || i == 6 || i == 26 || i == 30) continue;
    gpio_unused_cfg(i);
  }
}

/**
 * Apagado / desactivación máxima SIN dormir.
 * Llamar cuando ya terminaste BLE/sensores y antes de tu función de sleep.
 */
void deepPowerOffMostThings()
{
  // 1) SysTick OFF (evita ticks)
  SysTick->CTRL = 0;

  // 2) HFCLK OFF (si estuviera prendido)
  NRF_CLOCK->TASKS_HFCLKSTOP = 1;

  // 3) Flash QSPI externa a Deep Power Down
  // (si tu placa no tiene QSPI, comenta esto)
  g_flashTransport.begin();
  g_flashTransport.runCommand(0xB9); // Deep Power Down
  g_flashTransport.end();

  // 4) DC/DC ON (suele mejorar eficiencia)
  NRF_POWER->DCDCEN0 = 1;
  NRF_POWER->DCDCEN  = 1;

  // 5) USB OFF
#if defined(NRF_USBD)
  NRF_USBD->ENABLE = 0;
  NRF_USBD->USBPULLUP = 0;
#endif
  // Pines USB (P0.24, P0.25) a estado neutro
  NRF_GPIO->PIN_CNF[24] = 0;
  NRF_GPIO->PIN_CNF[25] = 0;

  // 6) NFC OFF
#if defined(NRF_NFCT)
  NRF_NFCT->TASKS_DISABLE = 1;
  NRF_NFCT->INTENCLR = 0xFFFFFFFF;
#endif

  // 7) UART OFF
#if defined(NRF_UARTE0)
  NRF_UARTE0->ENABLE = 0;
#endif
#if defined(NRF_UARTE1)
  NRF_UARTE1->ENABLE = 0;
#endif

  // 8) RNG OFF
#if defined(NRF_RNG)
  NRF_RNG->TASKS_STOP = 1;
  NRF_RNG->INTENCLR = 0xFFFFFFFF;
#endif

  // 9) RADIO OFF (por si acaso)
#if defined(NRF_RADIO)
  NRF_RADIO->TASKS_DISABLE = 1;
#endif

  // 10) CryptoCell OFF
#if defined(NRF_CRYPTOCELL)
  NRF_CRYPTOCELL->ENABLE = 0;
#endif

  // 11) Timers OFF
#if defined(NRF_TIMER1)
  NRF_TIMER1->TASKS_STOP = 1;
  NRF_TIMER2->TASKS_STOP = 1;
  NRF_TIMER3->TASKS_STOP = 1;
  NRF_TIMER4->TASKS_STOP = 1;
#endif

  // 12) RTC0/RTC1 OFF (deja RTC2 libre si tú lo usas para otra cosa)
#if defined(NRF_RTC0)
  NRF_RTC0->TASKS_STOP = 1;
#endif
#if defined(NRF_RTC1)
  NRF_RTC1->TASKS_STOP = 1;
#endif

  // 13) GPIOTE + PPI OFF
#if defined(NRF_GPIOTE)
  for (int i = 0; i < 8; i++) NRF_GPIOTE->CONFIG[i] = 0;
#endif
#if defined(NRF_PPI)
  NRF_PPI->CHEN = 0;
#endif

  // 14) LEDs en input (evita consumo por drive accidental)
#ifdef LED_BUILTIN
  pinMode(LED_BUILTIN, INPUT);
#endif

  // 15) GPIOs a estado seguro (ajusta rangos según tu MCU/board)
  disableGPIOs_range(0, 30);
  for (uint32_t i = 32; i <= 47; i++) {
    if (i == 41 || i == 42) continue;
    gpio_unused_cfg(i);
  }
}

} // namespace deepsleep

