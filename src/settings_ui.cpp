#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "settings_ui.h"
#include "config.h"

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                                OLED_MOSI, OLED_CLK, OLED_DC, OLED_RST, OLED_CS);

static bool oledOk = false;

int startHour = 0;
int startMinute = 0;
unsigned int durationMinutes = 240;
int offHour = 22;
int offMinute = 0;

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

static void drawRow(int y, const __FlashStringHelper* label, int h, int m, bool selected) {
  if (selected) {
    display.setTextSize(2);
    display.setCursor(0, y);
    display.print('>');
  }
  display.setTextSize(1);
  display.setCursor(14, y + 4);
  display.print(label);
 
  display.setTextSize(2);
  display.setCursor(64, y);
  printTime(h, m);
}


static void drawSettingsScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
 
  drawRow(0,  F("Start"),  startHour, startMinute, uiMode == UI_EDIT_START);
  drawRow(22, F("Max on"), durationMinutes / 60, durationMinutes % 60, uiMode == UI_EDIT_DURATION);
  drawRow(44, F("Off at"), offHour, offMinute, uiMode == UI_EDIT_OFFTIME);


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
  lastShownMode = -1; // force a first draw
  int lastPotVal = analogRead(POT_PIN);
 
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
