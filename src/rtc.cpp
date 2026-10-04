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

bool pastOffTime() {
  DateTime now = rtc.now();
  int nowMinutes = now.hour() * 60 + now.minute();
  int targetMinutes = offHour * 60 + offMinute;
  return nowMinutes >= targetMinutes;
}
