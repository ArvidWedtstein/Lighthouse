#pragma once

void setupRTC();

// Call once per loop(). Raises/clears FAULT_RTC depending on whether
// the RTC still answers on the I2C bus.
void updateRTCFault();

// Returns true while the RTC's current time is inside the daily window
// from the start time up to (not including) the off time.
//  - start < off: normal same-day window, e.g. 18:00 -> 23:00
//  - start > off: overnight window, e.g. 18:00 -> 02:00
//  - start == off: no restriction (always true)
// If the RTC is not responding, the window is treated as open
// (fail-open) so the light sensor alone still controls the relay.
// Start/off times come from settings_ui.h.
bool isWithinAllowedWindow();
 
