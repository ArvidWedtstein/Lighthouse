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