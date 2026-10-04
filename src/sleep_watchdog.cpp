#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "sleep_watchdog.h"

ISR(WDT_vect) {
  // Used as physical wakeup alarm clock.
}

void setupWatchdog() {
  MCUSR &= ~(1 << WDRF);               // Clear reset flag
  WDTCSR |= (1 << WDCE) | (1 << WDE);  // Enable watchdog configuration mode
  WDTCSR = (1 << WDP3) | (1 << WDP0);  // Set timeout window to 8.0 seconds
  WDTCSR |= (1 << WDIE) | (1 << WDE);  // Interrupt AND reset both armed
}

void enterDeepSleep() {
  ADCSRA &= ~(1 << ADEN); // ADC off 
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  sleep_cpu();

  sleep_disable();
  ADCSRA |= (1 << ADEN); // ADC back on


  wdt_reset();
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  WDTCSR = (1 << WDP3) | (1 << WDP0);
  WDTCSR |= (1 << WDIE) | (1 << WDE);
}

