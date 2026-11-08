#include <Arduino.h>
#include <avr/wdt.h>
#include "config.h"
#include "sleep_watchdog.h"
#include "light_sensor.h"
#include "relay_state.h"
#include "settings_ui.h"
#include "rtc.h"

void setup() {
  MCUSR = 0;
  wdt_disable();

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  setupSettingsUI();
  setupRTC();
  setupWatchdog();
}

void loop() {
  handleSettingsButton();

  updateLightSensor();
  updateRelayStateMachine();

  enterDeepSleep();
}


// I2C scanner - temporary replacement for src/main.cpp.
// Lists every device that answers on the I2C bus.
// Open the Serial Monitor at 9600 baud.
// #include <Arduino.h>
// #include <Wire.h>
 
// void setup() {
//   Serial.begin(9600);
//   Wire.begin();
//   delay(500);
//   Serial.println(F("Scanning I2C bus..."));
// }
 
// void loop() {
//   int found = 0;
 
//   for (uint8_t addr = 1; addr < 127; addr++) {
//     Wire.beginTransmission(addr);
//     if (Wire.endTransmission() == 0) {
//       Serial.print(F("Found device at 0x"));
//       if (addr < 16) Serial.print('0');
//       Serial.println(addr, HEX);
//       found++;
//     }
//   }
 
//   if (found == 0) {
//     Serial.println(F("No devices found - check SDA/SCL/power wiring"));
//   } else {
//     Serial.print(found);
//     Serial.println(F(" device(s) found"));
//   }
 
//   Serial.println();
//   delay(3000);
// }
