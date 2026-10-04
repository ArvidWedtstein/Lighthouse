#include <Arduino.h>
#include <TM1637Display.h>
#include "settings_ui.h"
#include "config.h"

static TM1637Display display(TM_CLK, TM_DIO);

unsigned int durationMinutes = 160;
int offHour = 23;
int offMinute = 0;

enum UIMode { UI_OFF, UI_EDIT_DURATION, UI_EDIT_OFFTIME };
static UIMode uiMode = UI_OFF;

static unsigned long lastInteractionMs = 0;
static bool lastButtonState = HIGH;
static unsigned long lastButtonChangeMs = 0;

static volatile bool buttonWoke = false;

static bool potTookOver = false;
static int lastUiMode = -1;

static void buttonISR() {
  buttonWoke = true;
}

// Returns true exactly once per confirmed, debounced button press.
static bool buttonPressed() {
  bool reading = digitalRead(BUTTON_PIN); 
  bool pressedEdge = false;
  if (reading != lastButtonState && (millis() - lastButtonChangeMs) > DEBOUNCE_MS) {
    lastButtonChangeMs = millis();
    if (reading == LOW) {
      pressedEdge = true;
    }
  }
  lastButtonState = reading;
  return pressedEdge;
}

static void updateDisplayForMode() {
  if (uiMode == UI_EDIT_DURATION) {
    int h = durationMinutes / 60;
    int m = durationMinutes % 60;
    display.showNumberDecEx(h * 100 + m, 0b11100000, true); // colon on
  } else if (uiMode == UI_EDIT_OFFTIME) {
    display.showNumberDecEx(offHour * 100 + offMinute, 0b11100000, true);
  } else {
    display.clear();
  }
}

static void runSettingsMode() {
  potTookOver = false;
  lastUiMode = uiMode;
  int lastPotVal = analogRead(POT_PIN); // baseline for movement detection

  while (uiMode != UI_OFF) {
    if (uiMode != lastUiMode) {
      potTookOver = false;
      lastUiMode = uiMode;
    }

    int potVal = analogRead(POT_PIN);

    // Treat meaningful knob movement as interaction, same as a button press,
    // so the idle timeout doesn't fire while someone's actively turning it.
    if (abs(potVal - lastPotVal) > 3) { 
      lastInteractionMs = millis();
      lastPotVal = potVal;
    }

    if (uiMode == UI_EDIT_DURATION) {
      unsigned int mapped = map(potVal, 0, 1023, DURATION_MIN, DURATION_MAX);
      mapped = (mapped / DURATION_STEP) * DURATION_STEP; 

      if (!potTookOver) {
        // Only take over once the knob has been moved close to the
        // current stored value
        int diff = abs((int)mapped - (int)durationMinutes);
        if (diff <= (int)DURATION_STEP) {
          potTookOver = true;
        }
      }
      if (potTookOver) {
        durationMinutes = mapped;
      }
    } else if (uiMode == UI_EDIT_OFFTIME) {
      int totalMinutes = map(potVal, 0, 1023, 0, 1439);
      totalMinutes = (totalMinutes / 15) * 15; 
      int currentTotal = offHour * 60 + offMinute;

      if (!potTookOver) {
        int diff = abs(totalMinutes - currentTotal);
        if (diff <= 15) {
          potTookOver = true;
        }
      }
      if (potTookOver) {
        offHour = totalMinutes / 60;
        offMinute = totalMinutes % 60;
      }
    }

    updateDisplayForMode();

    if (buttonPressed()) {
      lastInteractionMs = millis();
      if (uiMode == UI_EDIT_DURATION) {
        uiMode = UI_EDIT_OFFTIME;
      } else if (uiMode == UI_EDIT_OFFTIME) {
        uiMode = UI_OFF;
      }
    }

    if (millis() - lastInteractionMs > UI_TIMEOUT_MS) {
      uiMode = UI_OFF;
    }

    delay(100);
  }
  display.clear();
}

void setupSettingsUI() {
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, LOW);

  display.setBrightness(0x0f);
  display.clear();
}

void handleSettingsButton() {
  if (buttonWoke) {
    buttonWoke = false;
    delay(50);

    if (digitalRead(BUTTON_PIN) == LOW) {
      uiMode = UI_EDIT_DURATION;
      lastInteractionMs = millis();
      runSettingsMode(); 
      buttonWoke = false;
    }
  }
}
