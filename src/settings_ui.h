#pragma once

// ---- Live settings, adjustable via button + potentiometer ----
extern int startHour;                // earliest time the sensor may switch the relay on
extern int startMinute;              // default 00:00 = no early restriction
extern unsigned int durationMinutes; 
extern int offHour;       
extern int offMinute;       


void setupSettingsUI();

// Call once per loop() iteration, BEFORE updateLightSensor()/
// updateRelayStateMachine(). If a button press woke the chip, this
// blocks (runs its own fast loop, no deep sleep) until the user is
// done adjusting settings, then returns normally.
void handleSettingsButton();
