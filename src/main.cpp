#include <Arduino.h>
#include <avr/sleep.h> 
#include <avr/wdt.h> 
#include <Wire.h>
#include <RTClib.h>
#include <TM1637Display.h>

RTC_DS3231 rtc;

// ---- Pin definitions ----
const int RELAY_PIN = 6;
const int LIGHT_SENSOR_PIN = A0; 
const int LED_PIN = LED_BUILTIN; 

const int POT_PIN = A1;
const int BUTTON_PIN = 2;

const int TM_CLK = 4;
const int TM_DIO = 5;

TM1637Display display(TM_CLK, TM_DIO);

// ---- Settings ----
const int DARK_THRESHOLD = 150;  
const int LIGHT_THRESHOLD = 500;
const int DEBOUNCE_CYCLES = 4;

unsigned int durationMinutes = 120;  // default 2 hours
int offHour = 23;                    // default 11 PM
int offMinute = 0;

// ---- Adjustment ranges ----
const unsigned int DURATION_MIN = 30;    // 30 min
const unsigned int DURATION_MAX = 360;   // 6 hours
const unsigned int DURATION_STEP = 15;   // round to nearest 15 min

const unsigned long MAX_COOLDOWN_CYCLES = (12UL * 60UL * 60UL) / 8UL; 

// ---- Relay logic level ----
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;

// ---- State machine ----
enum State { IDLE, RUNNING, COOLDOWN };
State currentState = IDLE;
unsigned long cycleCounter = 0; 
unsigned long activeOnDurationCycles = 0; // captured at RUNNING start
bool isDark = false;

int darkStreak = 0;
int lightStreak = 0;

enum UIMode { UI_OFF, UI_EDIT_DURATION, UI_EDIT_OFFTIME };
UIMode uiMode = UI_OFF;
unsigned long lastInteractionMs = 0;
const unsigned long UI_TIMEOUT_MS = 15000; // 15s idle -> exit settings mode
bool lastButtonState = HIGH;
unsigned long lastButtonChangeMs = 0;
const unsigned long DEBOUNCE_MS = 250;

ISR(WDT_vect) {
  // Used as physical wakeup alarm clock
}

void setupWatchdog() {
  MCUSR &= ~(1 << WDRF); // Clear reset flag
  WDTCSR |= (1 << WDCE) | (1 << WDE); // Enable watchdog configuration mode
  WDTCSR = (1 << WDP3) | (1 << WDP0); // Set timeout window to 8.0 seconds
  WDTCSR |= (1 << WDIE) | (1 << WDE); // Enable watchdog interrupt mode AND reset both armed
}


void enterDeepSleep() {
  ADCSRA &= ~(1 << ADEN);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
  sleep_enable();
  sleep_cpu();
  sleep_disable(); 
  ADCSRA |= (1 << ADEN);
}

bool pastOffTime() {
  DateTime now = rtc.now();
  int nowMinutes = now.hour() * 60 + now.minute();
  int targetMinutes = offHour * 60 + offMinute;
  return nowMinutes >= targetMinutes;
}

bool buttonPressed() {
  bool reading = digitalRead(BUTTON_PIN); // LOW = pressed (pullup)
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


void updateDisplayForMode() {
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

void runSettingsMode() {
  while (uiMode != UI_OFF) {
    int potVal = analogRead(POT_PIN);

    if (uiMode == UI_EDIT_DURATION) {
      unsigned int mapped = map(potVal, 0, 1023, DURATION_MIN, DURATION_MAX);
      mapped = (mapped / DURATION_STEP) * DURATION_STEP; // round to nearest step
      durationMinutes = mapped;
    } else if (uiMode == UI_EDIT_OFFTIME) {
      int totalMinutes = map(potVal, 0, 1023, 0, 1439);
      totalMinutes = (totalMinutes / 15) * 15; // round to nearest 15 min
      offHour = totalMinutes / 60;
      offMinute = totalMinutes % 60;
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
      uiMode = UI_OFF; // idle timeout
    }

    delay(100); // responsive but not busy-spinning too hard
  }
  display.clear();
}

void setup() {
  MCUSR = 0;
  wdt_disable();

  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);

  display.setBrightness(0x0f);
  display.clear();

  Wire.begin();
  rtc.begin();

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //Serial.println("Initializing...");
  setupWatchdog();
}

void loop() {
  if (buttonPressed()) {
    uiMode = UI_EDIT_DURATION;
    lastInteractionMs = millis();
    runSettingsMode(); // blocks here (fast loop) until user is done
  }

  
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

  switch (currentState) {
    case IDLE:
      if (isDark) {
        currentState = RUNNING;
        cycleCounter = 0;
        activeOnDurationCycles = (durationMinutes * 60UL) / 8UL; // capture current setting
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
  
  enterDeepSleep();
}
