#include <Arduino.h>
#include "light_sensor.h"
#include "config.h"

bool isDark = false;

static int darkStreak = 0;
static int lightStreak = 0;

void updateLightSensor() {
  int lightLevel = analogRead(LIGHT_SENSOR_PIN);

  if (lightLevel <= DARK_THRESHOLD) {
    darkStreak++;
    lightStreak = 0;
  } else if (lightLevel >= LIGHT_THRESHOLD) {
    lightStreak++;
    darkStreak = 0;
  } else {
    darkStreak = 0;
    lightStreak = 0;
  }

  if (darkStreak >= DEBOUNCE_CYCLES) {
    isDark = true;
  } else if (lightStreak >= DEBOUNCE_CYCLES) {
    isDark = false;
  }
}
