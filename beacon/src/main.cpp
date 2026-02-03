#include <Arduino.h>
#include <bluefruit.h>
#include "Adafruit_SPIFlash.h"

// For SoftDevice-aware sleep
extern "C" uint32_t sd_app_evt_wait(void);

// LED pins
int loopCounter = 0;

// Example pressure value - dummy data for now, replace later with real datastream
uint16_t pressureValue = 1000;

// --- Setup external SPI flash ---
// We will set up the QSPI Flash here so it is ready to use later. Right now this
// program does not use the QSPI, but we will build in the low power handeling of
// flash during initial architecture as I am sure we will save something to flash later
#if defined(EXTERNAL_FLASH_USE_QSPI)
Adafruit_FlashTransport_QSPI flashTransport;
#elif defined(EXTERNAL_FLASH_USE_SPI)
Adafruit_FlashTransport_SPI flashTransport(EXTERNAL_FLASH_USE_CS, EXTERNAL_FLASH_USE_SPI);
#else
Adafruit_FlashTransport_SPI flashTransport;
#endif

Adafruit_SPIFlash flash(&flashTransport);

// ---------------------------------------------------------------------
// HELPER FUNCTIONS - Here wwe will gather everythign that we want to
// reuse on a regular basis.
// ---------------------------------------------------------------------

void deepPowerDownFlash() {
  //If the QSPI flash has been used, this will put it into a powered-down
  //state and save some microamps. Every bit counts!
  flash.begin();
  flashTransport.runCommand(0xB9);  // SPI deep power-down
  delay(10);
  flash.end();
}

void blinkLED(int ledPin, int times, int duration) {
  //lets be tidy and use a blink function any time a LED is being flashed
  //more than once. Single LED blinks might be handeled in line later so they don't
  //have additional delays. This could be re-written not using delay, but for this
  //project, the delays are really not impactful.
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, LOW);   // ON
    delay(duration);
    digitalWrite(ledPin, HIGH);  // OFF
    delay(duration);
  }
}

void setupLEDs() {
  // This could be in the main setup, don't know why I pulled it out here.
  // but I did.
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  // Turn all LEDs off initially
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
}

void updateBLEBeacon() {
  uint8_t manufData[2];
  manufData[0] = pressureValue & 0xFF;
  manufData[1] = (pressureValue >> 8) & 0xFF;

  // Stop first to safely update
  Bluefruit.Advertising.stop();
  Bluefruit.Advertising.clearData();
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.setName("PressurePoint");
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.addManufacturerData(manufData, sizeof(manufData));
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setFastTimeout(0);
  Bluefruit.Advertising.start();
}

// ---------------------------------------------------------------------
// STATES - These are our primary States for this simple Sensor.
// Currently states and helper functions really are not that
// different, just a mental map.
// ---------------------------------------------------------------------

void sendBLE() {
  //in this state we will simply get/maintain the value we want to
  //send to the BLE Beacon. in the future, this will get dynamic
  //i2c data from a sensor and fowrad it onto the update BLE Beacon function
  Serial.println("Entering State: BLE advertising");

  // Turn on Blue LED while updating BLE (comment this out later to save power)
  digitalWrite(LED_BLUE, LOW);

  // Increment dummy pressure value for testing
  pressureValue++;
  if (pressureValue > 1099) pressureValue = 1000;

  Serial.print("Updating BLE beacon: pressure = ");
  Serial.println(pressureValue);

  updateBLEBeacon();

  delay(250);  // allow a short broadcast period (can probably be shorter)
  digitalWrite(LED_BLUE, HIGH);
}

void onSleep() {
  //We will use onSleep() to try to put the system into the lowest
  //possible sleep state that we can while still being able to wake
  //back up with a time of some form and birng BLE back onlilne without
  //a reset. TARGET for this mode is 5-8 microamps but currently seeing
  //~125 microamps.

  // Blink green LED just before entering low-power
  digitalWrite(LED_GREEN, LOW);
  delay(150);
  digitalWrite(LED_GREEN, HIGH);

  // make sure all LEDs are off
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_GREEN, HIGH);

  // IF SPI Flash was running, Put SPI flash into deep power down
  deepPowerDownFlash();

  // Stop BLE advertising
  Bluefruit.Advertising.stop();

  // Disable the SoftDevice
  sd_softdevice_disable();

  // Sleep loop — no Serial, no delays (There is probably a better way to do this, fix later)
  const uint32_t STATE2_DURATION_MS = 5000;  // 5 seconds
  uint32_t start = millis();
  while (millis() - start < STATE2_DURATION_MS) {
    __WFI();  // CPU sleeps until an event occurs
  }

  // Blink green LED on wake (debug purposes, comment out later)
  digitalWrite(LED_GREEN, LOW);
  delay(150);
  digitalWrite(LED_GREEN, HIGH);

  // Re-enable BLE at the end of sleep. This should probably be pulled out elsewhere, like a general wakeup routine.
  Bluefruit.begin();
  Bluefruit.setTxPower(-8);
  Bluefruit.setName("PressurePoint");
  updateBLEBeacon();
}

void powerDown() {
  //we will use this as our lowest possible powerdown. You will need to
  //use the reset button to wake from this state, RAM will be lost.
  //this is currently measuring ~ 2.5 microamps. Which is good enough.
  Serial.println("Turning System OFF");

  //Blink the RED LED for debug purposes
  digitalWrite(LED_RED, LOW);
  delay(2000);
  digitalWrite(LED_RED, HIGH);

  // IF SPI Flash was running, Put SPI flash into deep power down (might not be neededd)
  deepPowerDownFlash();

  // make sure all LEDs are off
  digitalWrite(LED_RED,   HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE,  HIGH);

  //turn the processor off
  Serial.flush();
  NRF_POWER->SYSTEMOFF = 1;  // sub-µA sleep
  while (1) { }
}

// ---------------------------------------------------------------------
// SETUP & Main LOOP
// This is the main porition of our program, nice and simple as most of it
// is already worked out in the functions above.
// ---------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Setup start...");

  deepPowerDownFlash(); // its odd that this is here, but needed it for my first sleep cycle for some reason.
  setupLEDs();

  // Initialize BLE
  Bluefruit.autoConnLed(false);   // Disable default Blue LED for BLE - I don't want this behaviour right now
  Bluefruit.begin();
  Bluefruit.setTxPower(-8);
  Bluefruit.setName("PressurePoint");

  // Startup blink
  blinkLED(LED_RED, 1, 200);
  blinkLED(LED_GREEN, 1, 200);
  blinkLED(LED_BLUE, 1, 200);
  Serial.println("Setup complete.");
}

void loop() {
  Serial.print("Loop counter: ");
  Serial.println(loopCounter);
  sendBLE();  // BLE advertising
  onSleep();  // Ultra-low-power sleep
  loopCounter++; // just have this here for debug purposes and see when the RAM is reset
}
