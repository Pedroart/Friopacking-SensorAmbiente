#define DEEPSLEEP_WDT_TIMEOUT_S 10
#define DEEPSLEEP_DISABLE_USBD 0
#define DEEPSLEEP_BLINK_MS 0

#include <Arduino.h>
#include "deepsleep.h"

void setup() {
  deepsleep::Config cfg;

  deepsleep::startLfclk();
  deepsleep::startWdt(cfg);
  deepsleep::sleepForeverWfe();
}

void loop() {}
