#pragma once

// ============================================================
// Pin definitions
// ============================================================
const int RELAY_PIN        = 6;
const int LIGHT_SENSOR_PIN = A0;
const int LED_PIN          = LED_BUILTIN;

const int POT_PIN    = A1;
const int BUTTON_PIN = 2; 

// ============================================================
// OLED display (4-pin I2C module - shares SDA=A4 / SCL=A5 with the RTC)
// ============================================================
const int SCREEN_WIDTH  = 128;
const int SCREEN_HEIGHT = 64;
const int OLED_RESET    = -1;    // no reset pin on 4-pin modules
const uint8_t OLED_ADDR = 0x3C;

const uint32_t OLED_I2C_CLOCK = 100000UL;


// ============================================================
// Light sensor settings
// ============================================================
const int DARK_THRESHOLD   = 150;
const int LIGHT_THRESHOLD  = 500;
const int DEBOUNCE_CYCLES  = 8;  

// ============================================================
// Relay timing
// ============================================================
// Each watchdog cycle is ~8 seconds.
const unsigned long MAX_COOLDOWN_CYCLES = (12UL * 60UL * 60UL) / 8UL; // safetly fallback // 12h

// ============================================================
// Adjustable duration / off-time ranges (via settings UI)
// ============================================================
const unsigned int DURATION_MIN  = 30;  
const unsigned int DURATION_MAX  = 360; 
const unsigned int DURATION_STEP = 15;  


// ============================================================
// Default settings (used at every power-up/reset; the settings UI
// can change them while running, but they are not saved)
// ============================================================
// Earliest time the light sensor is allowed to switch the relay on.
// 00:00 would mean "no restriction", so pick the earliest time you'd
// ever want the light on.
const int DEFAULT_START_HOUR    = 14;
const int DEFAULT_START_MINUTE  = 0;
const unsigned int DEFAULT_DURATION_MINUTES = 300; // max on time
const int DEFAULT_OFF_HOUR      = 22;              // relay always off from here
const int DEFAULT_OFF_MINUTE    = 0;

// ============================================================
// Status LED (onboard LED)
// ============================================================
const bool STATUS_HEARTBEAT = true;       // one very short blink per wake cycle when all is well
const bool SIGNAL_DISPLAY_FAULT = true;   // include "display not responding" in the fault codes
const unsigned int STATUS_HEARTBEAT_MS = 20;
const unsigned int STATUS_BLINK_ON_MS  = 200;
const unsigned int STATUS_BLINK_OFF_MS = 200;

// ============================================================
// Relay logic level
// ============================================================
const int RELAY_ON  = LOW;
const int RELAY_OFF = HIGH;

// ============================================================
// Settings UI behavior
// ============================================================
const unsigned long UI_TIMEOUT_MS = 15000; 
const unsigned long DEBOUNCE_MS   = 250;  
