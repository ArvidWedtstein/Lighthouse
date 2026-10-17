#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "settings_ui.h"
#include "config.h"

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                                OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

static bool oledOk = false;

unsigned int durationMinutes = 240;
int offHour = 22;
int offMinute = 0;

enum UIMode { UI_OFF, UI_EDIT_DURATION, UI_EDIT_OFFTIME };
static UIMode uiMode = UI_OFF;

static unsigned long lastInteractionMs = 0;
static bool lastButtonState = HIGH;
static unsigned long lastButtonChangeMs = 0;

static volatile bool buttonWoke = false;

static bool potTookOver = false;
static int lastUiMode = -1;

static int lastShownMode = -1;
static unsigned int lastShownDuration = 0;
static int lastShownOffTotal = -1;

static void buttonISR() {
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

// Helpers
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

static void drawSettingsScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
 
  // ---- Row 1: max on time ----
  display.setTextSize(1);
  display.setCursor(18, 0);
  display.print(F("Max on time"));
  display.setTextSize(2);
  display.setCursor(18, 10);
  printTime(durationMinutes / 60, durationMinutes % 60);
 
  // ---- Row 2: off time ----
  display.setTextSize(1);
  display.setCursor(18, 36);
  display.print(F("Turn off at"));
  display.setTextSize(2);
  display.setCursor(18, 46);
  printTime(offHour, offMinute);
 
  // ---- Marker showing which setting the knob is editing ----
  display.setTextSize(2);
  if (uiMode == UI_EDIT_DURATION) {
    display.setCursor(0, 10);
    display.print('>');
  } else if (uiMode == UI_EDIT_OFFTIME) {
    display.setCursor(0, 46);
    display.print('>');
  }
 
  display.display();
}

static void refreshDisplayIfNeeded() {
  if (!oledOk) return;
 
  int offTotal = offHour * 60 + offMinute;
  if (uiMode == lastShownMode &&
      durationMinutes == lastShownDuration &&
      offTotal == lastShownOffTotal) {
    return;
  }
 
  drawSettingsScreen();
  lastShownMode = uiMode;
  lastShownDuration = durationMinutes;
  lastShownOffTotal = offTotal;
}



static void runSettingsMode() {
  potTookOver = false;
  lastUiMode = uiMode;
  lastShownMode = -1; // force a first draw
  int lastPotVal = analogRead(POT_PIN); // baseline for movement detection
 
  oledWake();
 
  while (uiMode != UI_OFF) {
    if (uiMode != lastUiMode) {
      potTookOver = false;
      lastUiMode = uiMode;
    }
 
    int potVal = analogRead(POT_PIN);
 

    if (abs(potVal - lastPotVal) > 3) { 
      lastInteractionMs = millis();
      lastPotVal = potVal;
    }
 
    if (uiMode == UI_EDIT_DURATION) {
      unsigned int mapped = map(potVal, 0, 1023, DURATION_MIN, DURATION_MAX);
      mapped = (mapped / DURATION_STEP) * DURATION_STEP; 
 
      if (!potTookOver) {
        // Only take over once the knob has been moved close to the
        // current stored value - avoids a jump when switching settings.
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
 
    refreshDisplayIfNeeded();
 
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
 
  oledSleep();
}

void setupSettingsUI() {

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, LOW);
 
  // begin() also starts the I2C bus. Returns false if the framebuffer
  // (1KB of the Uno's 2KB RAM) couldn't be allocated.
  oledOk = display.begin(SSD1306_SWITCHCAPVCC);
  oledSleep();
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
