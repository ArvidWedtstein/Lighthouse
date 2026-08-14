#include <Arduino.h>
#include <avr/sleep.h> 
#include <avr/wdt.h> 

// ---- Pin definitions ----
const int RELAY_PIN = 6;
const int LIGHT_SENSOR_PIN = A0; 
const int LED_PIN = LED_BUILTIN; 

// ---- Settings ----
const int DARK_THRESHOLD = 150;  
const int LIGHT_THRESHOLD = 500;

const unsigned long ON_DURATION_CYCLES   = (2UL * 60UL * 60UL) / 8UL;   
const unsigned long MAX_COOLDOWN_CYCLES = (12UL * 60UL * 60UL) / 8UL; 

// ---- Debounce settings ----
// Each cycle is ~8s (one watchdog wake). Requiring 4 consecutive consistent
// readings means a transition needs to hold for ~32s before it's trusted.
const int DEBOUNCE_CYCLES = 4;

// ---- Relay logic level ----
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;

// ---- State machine ----
enum State {
  IDLE,     
  RUNNING,  
  COOLDOWN        // Relay off, waiting for light before re-arming
};

State currentState = IDLE;
unsigned long cycleCounter = 0; 
bool isDark = false;
int ledState = LOW;

int darkStreak = 0;
int lightStreak = 0;

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

   // "Pet" the watchdog: re-arm interrupt mode so the next timeout wakes us
  // normally instead of resetting. If loop() ever hangs and never reaches
  // this point again, the watchdog will reset the chip instead.
  wdt_reset();
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  WDTCSR = (1 << WDP3) | (1 << WDP0);
  WDTCSR |= (1 << WDIE) | (1 << WDE);
}

void setup() {
  MCUSR = 0;
  wdt_disable(); 

  //Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF);


  //Serial.println("Initializing...");
  setupWatchdog();
}

void loop() {
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

  /*Serial.println("Light: ");
  Serial.print(lightLevel);
  Serial.println("Filtered Status: ");
  Serial.print(isDark ? "DARK" : "LIGHT");
  Serial.println("State: ");
  switch (currentState) {
    case IDLE:     Serial.print("IDLE"); break;
    case RUNNING:  Serial.print("RUNNING"); break;
    case COOLDOWN: Serial.print("COOLDOWN"); break;
  }*/

  switch (currentState) {
    case IDLE:
      if (isDark) {
        currentState = RUNNING;
        cycleCounter = 0;
        digitalWrite(RELAY_PIN, RELAY_ON);
        //Serial.println("Dark detected - Relay ON");
      }
      break;

    case RUNNING:
      cycleCounter++;
      if (cycleCounter >= ON_DURATION_CYCLES) {
        digitalWrite(RELAY_PIN, RELAY_OFF);
        currentState = COOLDOWN;
        cycleCounter = 0;
        //Serial.println("time elapsed - Relay OFF, waiting for light");
      }
      break;

    case COOLDOWN:
      cycleCounter++;
      if (!isDark || cycleCounter >= MAX_COOLDOWN_CYCLES) {
        currentState = IDLE;
        cycleCounter = 0;
        //Serial.println("Re-armed");
      }
      break;
  }

  if (isDark && currentState == IDLE) {
    ledState = HIGH;
  } else {
    ledState = LOW;
  }
  digitalWrite(LED_PIN, ledState);

  enterDeepSleep();
}
