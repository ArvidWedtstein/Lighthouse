#pragma once
#include <Arduino.h>
 
// Fault codes. The value is also the number of blinks shown on the LED.
// Lower number = higher priority (only the highest-priority active
// fault is shown each cycle).
enum FaultCode : uint8_t {
  FAULT_RTC     = 2,  // clock not responding, or its time was reset
  FAULT_SENSOR  = 3,  // never saw light for the whole cooldown period
  FAULT_DISPLAY = 4   // OLED not responding on the I2C bus
};
 
// Call once from setup(), right after wdt_disable().
// Flashes the LED rapidly 5 times so you can see every (re)start.
void setupStatusLed();
 
// Mark a fault active/cleared from anywhere in the code.
void setFault(FaultCode code, bool active);
 
// Call once per loop() iteration, just before going to sleep.
// Faults: blinks the fault's code. No faults: one very short heartbeat
// blink (if STATUS_HEARTBEAT is enabled in config.h).
void showStatusLed();


void flashPressAck();