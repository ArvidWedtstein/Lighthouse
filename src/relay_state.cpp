#include <Arduino.h>
#include "relay_state.h"
#include "light_sensor.h"
#include "rtc.h"
#include "settings_ui.h"
#include "config.h"

RelayState currentState = IDLE;

static unsigned long cycleCounter = 0;
static unsigned long activeOnDurationCycles = 0; 

void updateRelayStateMachine() {
  switch (currentState) {

    case IDLE:
      if (isDark) {
        currentState = RUNNING;
        cycleCounter = 0;

        activeOnDurationCycles = (durationMinutes * 60UL) / 8UL;
        digitalWrite(RELAY_PIN, RELAY_ON);
      }
      break;

    case RUNNING:
      cycleCounter++;
      if (cycleCounter >= activeOnDurationCycles || pastOffTime()) {
        digitalWrite(RELAY_PIN, RELAY_OFF);
        currentState = COOLDOWN;
        cycleCounter = 0;
      }
      break;

    case COOLDOWN:
      cycleCounter++;
      if (!isDark || cycleCounter >= MAX_COOLDOWN_CYCLES) {
        currentState = IDLE;
        cycleCounter = 0;
      }
      break;
  }

  digitalWrite(LED_PIN, (isDark && currentState == IDLE) ? HIGH : LOW);
}
