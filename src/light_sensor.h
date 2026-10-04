#pragma once

// Debounced dark/light status. True once DEBOUNCE_CYCLES consecutive
// readings have confirmed darkness; false once DEBOUNCE_CYCLES
// consecutive readings have confirmed light.
extern bool isDark;

void updateLightSensor();
