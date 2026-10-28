#pragma once

void setupRTC();

// Returns true while the RTC's current time is inside the daily window
// from the start time up to (not including) the off time.
//  - start < off: normal same-day window, e.g. 18:00 -> 23:00
//  - start > off: overnight window, e.g. 18:00 -> 02:00
//  - start == off: no restriction (always true)
// Start/off times come from settings_ui.h.
bool isWithinAllowedWindow();
 
