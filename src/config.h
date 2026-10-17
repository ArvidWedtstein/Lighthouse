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
// OLED display (I2C - shares SDA=A4 / SCL=A5 with the RTC)
// ============================================================
const int SCREEN_WIDTH  = 128;
const int SCREEN_HEIGHT = 64;
const int OLED_CLK  = 4;   // module pin "D0"
const int OLED_MOSI = 5;   // module pin "D1"
const int OLED_DC   = 7;
const int OLED_RST  = 8;
const int OLED_CS   = 9;


// ============================================================
// Light sensor settings
// ============================================================
const int DARK_THRESHOLD   = 150;
const int LIGHT_THRESHOLD  = 500;
const int DEBOUNCE_CYCLES  = 4;  

// ============================================================
// Relay timing
// ============================================================
// Each watchdog cycle is ~8 seconds.
const unsigned long MAX_COOLDOWN_CYCLES = (12UL * 60UL * 60UL) / 8UL; // safetly fallback

// ============================================================
// Adjustable duration / off-time ranges (via settings UI)
// ============================================================
const unsigned int DURATION_MIN  = 30;  
const unsigned int DURATION_MAX  = 360; 
const unsigned int DURATION_STEP = 15;  

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
