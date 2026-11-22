#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include "rtc.h"
#include "settings_ui.h"
#include "status_led.h"

static RTC_DS3231 rtc;
static const uint8_t RTC_ADDR = 0x68;
static bool rtcTimeUntrusted = false;

static bool rtcResponding() {
  Wire.beginTransmission(RTC_ADDR);
  return Wire.endTransmission() == 0;
}

void setupRTC() {
  Wire.begin();

  if (!rtc.begin()) {
    setFault(FAULT_RTC, true);
    return;
  }

  if (rtc.lostPower()) {
    // Battery died or very first boot: time is just the compile time,
    // so flag it until the next clean boot.
    rtcTimeUntrusted = true;
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  setFault(FAULT_RTC, rtcTimeUntrusted);
}

void updateRTCFault() {
  setFault(FAULT_RTC, rtcTimeUntrusted || !rtcResponding());
}

bool isWithinAllowedWindow() {
  // A dead RTC would return all-zero times (00:00) and the relay would
  // then never switch on. Fail open instead; FAULT_RTC tells you.
  if (!rtcResponding()) return true;
  
  DateTime now = rtc.now();
  int nowMin   = now.hour() * 60 + now.minute();
  int startMin = startHour * 60 + startMinute;
  int offMin   = offHour * 60 + offMinute;
 
  if (startMin == offMin) return true;  
  if (startMin < offMin)  return nowMin >= startMin && nowMin < offMin;
  return nowMin >= startMin || nowMin < offMin;
}

