#include "config.hpp"

#include "timer_led.hpp"

hw_timer_t *loop_timer = NULL;

volatile bool timerFired = false;

// używajcie IRAM_ATTR z przerwaniami bo tak program lepiej działa, jak nie użyjecie tego to jeden li-pol na świcie wybucha
void IRAM_ATTR onLedTimerBlink()
{
  digitalWrite(USER_LED, !digitalRead(USER_LED));
  timerFired = true;
}

void initLedTimer()
{
  loop_timer = timerBegin(0, 80, true);
  timerAttachInterrupt(loop_timer, &onLedTimerBlink, true);

  stopLedTimer();
  // odpowiada miganiu co 10 ms
  // timerAlarmWrite(loop_timer,10000,true);
  // timerAlarmEnable(loop_timer);
}

void setLedTimerTime(float seconds)
{
  timerAlarmWrite(loop_timer, static_cast<uint64_t>(1000000 * seconds), true);
  timerAlarmEnable(loop_timer);
}

void startLedTimer()
{
  timerStart(loop_timer);
  digitalWrite(USER_LED, LOW);
}

void stopLedTimer()
{
  timerStop(loop_timer);
  timerAlarmDisable(loop_timer);
  timerRestart(loop_timer);
  digitalWrite(USER_LED, LOW);
}

void resetLedTimer()
{
  stopLedTimer();
  startLedTimer();
}
