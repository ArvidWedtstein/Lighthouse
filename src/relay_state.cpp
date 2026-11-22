#include <Arduino.h>
#include "relay_state.h"
#include "light_sensor.h"
#include "rtc.h"
#include "settings_ui.h"
#include "config.h"
#include "status_led.h"

RelayState currentState = IDLE;

static unsigned long cycleCounter = 0;
static unsigned long activeOnDurationCycles = 0; 

void updateRelayStateMachine() {
  // A confirmed "light" reading proves the sensor can still see changes.
  if (!isDark) {
    setFault(FAULT_SENSOR, false);
  }

  switch (currentState) {
    
    case IDLE:
      // isDark is checked first so the RTC is only read when it matters.
      // If it's dark before the start time, we simply keep waiting here
      // and switch on the moment the window opens.
      if (isDark && isWithinAllowedWindow()) {
        currentState = RUNNING;
        cycleCounter = 0;
        // Capture the current duration setting now - turning the knob
        // mid-cycle won't retroactively change an already-running timer.
        activeOnDurationCycles = (durationMinutes * 60UL) / 8UL;
        digitalWrite(RELAY_PIN, RELAY_ON);
      }
      break;

    case RUNNING:
      cycleCounter++;
      if (cycleCounter >= activeOnDurationCycles || !isWithinAllowedWindow()) {
        digitalWrite(RELAY_PIN, RELAY_OFF);
        currentState = COOLDOWN;
        cycleCounter = 0;
      }
      break;

    case COOLDOWN:
      cycleCounter++;
      if (!isDark) {
        currentState = IDLE;
        cycleCounter = 0;
      } else if (cycleCounter >= MAX_COOLDOWN_CYCLES) {
        // Safety fallback fired: it stayed "dark" for the whole
        // cooldown period without ever seeing light. Sensor is
        // probably stuck, covered, or disconnected.
        setFault(FAULT_SENSOR, true);
        currentState = IDLE;
        cycleCounter = 0;
      }
      break;
  }
}
