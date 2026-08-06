#include <Arduino.h>

// ---- Pin definitions ----
const int RELAY_PIN = 6;
const int LIGHT_SENSOR_PIN = A0; 

// ---- Settings ----
int darkThreshold = 300;  
const int LoopDelay = 10000; // 10s

const unsigned long ON_DURATION = 3UL * 60UL * 60UL * 1000UL; 

// ---- Relay logic level ----
const int RELAY_ON = LOW;
const int RELAY_OFF = HIGH;

// ---- State tracking ----
bool relayActive = false;
unsigned long relayStartTime = 0;

bool ActivateRelay = false;


void setup() {
  Serial.begin(9600);

  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, RELAY_OFF); 

}

void loop() {

  int lightLevel = analogRead(LIGHT_SENSOR_PIN);
  Serial.println(lightLevel);

  if (!relayActive) {
    if (lightLevel < darkThreshold) {
      relayActive = true;
      relayStartTime = millis();
      digitalWrite(RELAY_PIN, RELAY_ON);
      Serial.println("Dark detected -> Relay ON");
    }
  } else {
    // Relay is on, check if 3 hours have passed

    Serial.println(millis() - relayStartTime);
    if (millis() - relayStartTime >= ON_DURATION) {
      relayActive = false;
      digitalWrite(RELAY_PIN, RELAY_OFF);
      Serial.println("3 hours elapsed -> Relay OFF");
    }
  }

  delay(LoopDelay);
}
