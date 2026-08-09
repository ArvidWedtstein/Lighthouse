#include <Arduino.h>

bool serialActive = false;

// ---- Pin definitions ----
const int RELAY_PIN = 6;
const int LIGHT_SENSOR_PIN = A0; 

// ---- Settings ----
int darkThreshold = 5;  
const unsigned long LoopDelay = 1000UL; // 1s

const unsigned long ON_DURATION = 2UL * 60UL * 60UL * 1000UL; 
const unsigned long MAX_COOLDOWN = 12UL * 60UL * 60UL * 1000UL;  

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
unsigned long relayStartTime = 0;
unsigned long cooldownStartTime = 0;


void setup() {
  Serial.begin(9600);

  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, RELAY_OFF);

  Serial.println("Press [ENTER] or send any character to begin debugging...");
}

void loop() {
  if (Serial.available() > 0) {
    serialActive = true; 
    while(Serial.available() > 0) { Serial.read(); } // Clear the buffer
  }

  int lightLevel = analogRead(LIGHT_SENSOR_PIN);
  bool isDark = (lightLevel < darkThreshold);

  if (serialActive) {
    Serial.print("Light: ");
    Serial.print(lightLevel);
    Serial.print(" | State: ");
    Serial.println(currentState);
  }

  switch (currentState) {
    case IDLE:
      if (isDark) {
        currentState = RUNNING;
        relayStartTime = millis();
        digitalWrite(RELAY_PIN, RELAY_ON);
        Serial.println("Dark detected -> Relay ON");
      }
      break;
    case RUNNING:
      if (millis() - relayStartTime >= ON_DURATION) {
        digitalWrite(RELAY_PIN, RELAY_OFF);
        currentState = COOLDOWN;
        cooldownStartTime = millis();
        Serial.println("time elapsed -> Relay OFF, waiting for light");
      }
      break;
    case COOLDOWN:
      if (!isDark) {
        currentState = IDLE;
        Serial.println("Light detected -> Re-armed, watching for darkness");
      } else if (millis() - cooldownStartTime >= MAX_COOLDOWN) {
        currentState = IDLE;
        Serial.println("Cooldown timeout reached -> Force re-armed (safety fallback)");
      }
      break;
  }

  delay(LoopDelay);
}
