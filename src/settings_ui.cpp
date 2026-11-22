#include <Arduino.h>
#include <avr/wdt.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h> 
#include "settings_ui.h"
#include "config.h"
#include "status_led.h"


static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
static bool oledOk = false;

int startHour = DEFAULT_START_HOUR;
int startMinute = DEFAULT_START_MINUTE;
unsigned int durationMinutes = DEFAULT_DURATION_MINUTES;
int offHour = DEFAULT_OFF_HOUR;
int offMinute = DEFAULT_OFF_MINUTE;

enum UIMode { UI_OFF, UI_EDIT_START, UI_EDIT_DURATION, UI_EDIT_OFFTIME };
static UIMode uiMode = UI_OFF;

static unsigned long lastInteractionMs = 0;
static bool lastButtonState = HIGH;
static unsigned long lastButtonChangeMs = 0;

static volatile bool buttonWoke = false;

static bool potTookOver = false;
static int lastUiMode = -1;

static int lastShownMode = -1;
static int lastShownStartTotal = -1;
static unsigned int lastShownDuration = 0;
static int lastShownOffTotal = -1;

static void buttonISR() {
  // A LOW-level interrupt re-fires continuously while the pin stays LOW.
  // That starves the main program AND Timer0, so delay()/millis() stop
  // working while the button is held. Detach here so it fires exactly
  // once per press; rearmButtonWake() re-attaches it after release.
  detachInterrupt(digitalPinToInterrupt(BUTTON_PIN));
  buttonWoke = true;
}

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

// ===== { OLED Helpers } =====

// begin() can't tell whether a display is actually connected, so probe
// the I2C address directly: does anything answer there?
static bool displayResponding() {
  Wire.beginTransmission(OLED_ADDR);
  return Wire.endTransmission() == 0;
}

static void oledWake() {
  if (!oledOk) return;
  display.ssd1306_command(SSD1306_DISPLAYON);
}

static void oledSleep() {
  if (!oledOk) return;
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
}

static void printTime(int h, int m) {
  if (h < 10) display.print('0');
  display.print(h);
  display.print(':');
  if (m < 10) display.print('0');
  display.print(m);
}

static void drawRow(int y, const __FlashStringHelper* label, int h, int m, bool selected) {
  if (selected) {
    display.fillRoundRect(0, y + 1, 5, 12, 6, SSD1306_WHITE);
  }

  display.setCursor(8, y);
  display.print(label);
 
  display.setCursor(80, y);
  printTime(h, m);
}

static void drawSettingsScreen() {
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(SSD1306_WHITE);
 
  drawRow(16,  F("Start"),  startHour, startMinute, uiMode == UI_EDIT_START);
  drawRow(38, F("Max on"), durationMinutes / 60, durationMinutes % 60, uiMode == UI_EDIT_DURATION);
  drawRow(60, F("Off at"), offHour, offMinute, uiMode == UI_EDIT_OFFTIME);


  display.display();
}

static void refreshDisplayIfNeeded() {
  if (!oledOk) return;
 
  int startTotal = startHour * 60 + startMinute;
  int offTotal = offHour * 60 + offMinute;

  if (uiMode == lastShownMode &&
      startTotal == lastShownStartTotal &&
      durationMinutes == lastShownDuration &&
      offTotal == lastShownOffTotal) {
    return;
  }

  drawSettingsScreen();
  lastShownMode = uiMode;
  lastShownStartTotal = startTotal;
  lastShownDuration = durationMinutes;
  lastShownOffTotal = offTotal;
}

// ===== { Knob tracking } =====

static void trackTimeOfDay(int potVal, int &hour, int &minute) {
  int total = map(potVal, 0, 1023, 0, 1439);
  total = (total / 15) * 15;
  int current = hour * 60 + minute;
 
  if (!potTookOver && abs(total - current) <= 15) {
    potTookOver = true;
  }
  if (potTookOver) {
    hour = total / 60;
    minute = total % 60;
  }
}

static void trackDuration(int potVal) {
  unsigned int mapped = map(potVal, 0, 1023, DURATION_MIN, DURATION_MAX);
  mapped = (mapped / DURATION_STEP) * DURATION_STEP;
 
  if (!potTookOver && abs((int)mapped - (int)durationMinutes) <= (int)DURATION_STEP) {
    potTookOver = true;
  }
  if (potTookOver) {
    durationMinutes = mapped;
  }
}


static void runSettingsMode() {
  potTookOver = false;
  lastUiMode = uiMode;
  lastShownMode = -1;
  int lastPotVal = analogRead(POT_PIN);
 
  oledWake();
 
  while (uiMode != UI_OFF) {
    // The watchdog is armed to reset the chip. Normally enterDeepSleep()
    // re-arms it each cycle, but this loop can run for 15+ seconds, so
    // feed it here or the board resets itself mid-adjustment.
    wdt_reset();

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


    if (uiMode == UI_EDIT_START) {
      trackTimeOfDay(potVal, startHour, startMinute);
    } else if (uiMode == UI_EDIT_DURATION) {
      trackDuration(potVal);
    } else if (uiMode == UI_EDIT_OFFTIME) {
      trackTimeOfDay(potVal, offHour, offMinute);
    }

    refreshDisplayIfNeeded();

    if (buttonPressed()) {
      lastInteractionMs = millis();
      if (uiMode == UI_EDIT_START) {
        uiMode = UI_EDIT_DURATION;
      } else if (uiMode == UI_EDIT_DURATION) {
        uiMode = UI_EDIT_OFFTIME;
      } else if (uiMode == UI_EDIT_OFFTIME) {
        uiMode = UI_OFF;
      }
    }
 
    if (millis() - lastInteractionMs > UI_TIMEOUT_MS) {
      uiMode = UI_OFF; // idle timeout
    }
 
    delay(100);
  }
 
  oledSleep();
}

void setupSettingsUI() {
  // NOTE: LOW level trigger is required (not FALLING) so the button can
  // actually wake the chip from SLEEP_MODE_PWR_DOWN - edge-triggered
  // interrupts need the I/O clock running, which is stopped during
  // deep sleep.
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, LOW);
 
  // begin() also starts the I2C bus. Returns false if the framebuffer
  // (1KB of the Uno's 2KB RAM) couldn't be allocated.
  oledOk = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  if (SIGNAL_DISPLAY_FAULT) {
    setFault(FAULT_DISPLAY, !oledOk || !displayResponding());
  }
  oledSleep();
}

// Blocks until the button is released. Times out so a stuck button
// can't hang the device forever.
static void waitForRelease() {
  unsigned long start = millis();
  while (digitalRead(BUTTON_PIN) == LOW && millis() - start < 3000) {
    delay(10);
  }
  delay(50); 
}

static void rearmButtonWake() {
  buttonWoke = false;
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, LOW);
}


void handleSettingsButton() {
  if (!buttonWoke) return;

  buttonWoke = false;
  delay(50); // debounce - Timer0 runs normally now that the ISR has detached

  if (digitalRead(BUTTON_PIN) == LOW) {
    waitForRelease();

    lastButtonState = HIGH;
    lastButtonChangeMs = millis();

    if (SIGNAL_DISPLAY_FAULT) {
      setFault(FAULT_DISPLAY, !displayResponding()); // re-check whenever settings open
    }

    uiMode = UI_EDIT_START;
    lastInteractionMs = millis();
    runSettingsMode(); 
  }

  waitForRelease();
  rearmButtonWake();
}
