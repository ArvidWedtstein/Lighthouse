#include <Arduino.h>
#include "status_led.h"
#include "config.h"

static uint8_t faultMask = 0; // bit N set = fault code N active

static void blinkTimes(uint8_t count, unsigned int onMs, unsigned int offMs) {
  for (uint8_t i = 0; i < count; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(onMs);
    digitalWrite(LED_PIN, LOW);
    if (i + 1 < count) delay(offMs);
  }
}

void setupStatusLed() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  blinkTimes(5, 60, 60); // rapid flashes = the board just (re)started
  delay(500);
}

void setFault(FaultCode code, bool active) {
  if (active) {
    faultMask |= (1 << code);
  } else {
    faultMask &= ~(1 << code);
  }
}

void showStatusLed() {
  // Ascending code order = priority order (RTC first)
  for (uint8_t code = FAULT_RTC; code <= FAULT_DISPLAY; code++) {
    if (faultMask & (1 << code)) {
      blinkTimes(code, STATUS_BLINK_ON_MS, STATUS_BLINK_OFF_MS);
      return;
    }
  }

  if (STATUS_HEARTBEAT) {
    blinkTimes(1, STATUS_HEARTBEAT_MS, 0);
  }
}

void flashPressAck() {
  blinkTimes(2, 50, 50);
}