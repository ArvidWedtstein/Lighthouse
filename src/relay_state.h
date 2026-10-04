#pragma once

enum RelayState { IDLE, RUNNING, COOLDOWN };
extern RelayState currentState;

void updateRelayStateMachine();
