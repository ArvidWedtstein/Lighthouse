#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include "rtc.h"
#include "settings_ui.h"

static RTC_DS3231 rtc;

void setupRTC() {
  Wire.begin();
  rtc.begin();

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

bool isWithinAllowedWindow() {
  DateTime now = rtc.now();
  int nowMin   = now.hour() * 60 + now.minute();
  int startMin = startHour * 60 + startMinute;
  int offMin   = offHour * 60 + offMinute;
 
  if (startMin == offMin) return true;  
  if (startMin < offMin)  return nowMin >= startMin && nowMin < offMin;
  return nowMin >= startMin || nowMin < offMin;
}

