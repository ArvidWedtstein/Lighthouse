#include <Arduino.h>
#include <avr/wdt.h>
#include "config.h"
#include "sleep_watchdog.h"
#include "light_sensor.h"
#include "relay_state.h"
#include "settings_ui.h"
#include "rtc.h"
#include "status_led.h"

void setup() {
  MCUSR = 0;
  wdt_disable();

  setupStatusLed();

  pinMode(RELAY_PIN, OUTPUT);
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

  updateRTCFault();
  showStatusLed(); // heartbeat or fault code, always ends with the LED off

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


// OLED triage test - temporary replacement for src/main.cpp.
// Runs the display at 100 kHz (the library default is 400 kHz) and tries
// both VCC init modes. Open the Serial Monitor at 9600 baud and WATCH
// THE SCREEN while each stage runs.
// #include <Arduino.h>
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// const uint8_t OLED_ADDR = 0x3C;

// // Last two arguments: bus clock during and after transfers (100 kHz)
// Adafruit_SSD1306 display(128, 64, &Wire, -1, 100000UL, 100000UL);

// static void drawTest(const __FlashStringHelper* label) {
//   // 1) Every pixel on: if the panel works at all, you'll see a white screen
//   display.clearDisplay();
//   display.fillRect(0, 0, 128, 64, SSD1306_WHITE);
//   display.display();
//   delay(2000);

//   // 2) Border + text: shows the true screen edges and any shift/garbling
//   display.clearDisplay();
//   display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
//   display.setTextSize(2);
//   display.setTextColor(SSD1306_WHITE);
//   display.setCursor(8, 8);
//   display.println(label);
//   display.display();
//   delay(4000);
// }

// void setup() {
//   Serial.begin(9600);
//   Wire.begin();

//   Wire.beginTransmission(OLED_ADDR);
//   Serial.print(F("Probe 0x3C: "));
//   Serial.println(Wire.endTransmission() == 0 ? F("ACK") : F("no answer"));

//   Serial.println(F("Stage A: SWITCHCAPVCC (right for almost all modules)"));
//   if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
//     Serial.println(F("begin() failed (no ACK or out of RAM)"));
//     while (true) { delay(1000); }
//   }
//   drawTest(F("Stage A"));

//   Serial.println(F("Stage B: EXTERNALVCC"));
//   display.begin(SSD1306_EXTERNALVCC, OLED_ADDR);
//   drawTest(F("Stage B"));

//   Serial.println(F("Done."));
// }

// void loop() {
// }