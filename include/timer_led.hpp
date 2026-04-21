#pragma once

#include <Arduino.h>

extern hw_timer_t *loop_timer;

extern volatile bool timerFired;

// używajcie IRAM_ATTR z przerwaniami bo tak program lepiej działa, jak nie użyjecie tego to jeden li-pol na świcie wybucha
void IRAM_ATTR onLedTimerBlink();

void initLedTimer();

void setLedTimerTime(float seconds);

void startLedTimer();

void stopLedTimer();

void resetLedTimer();
