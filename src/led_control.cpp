#include "led_control.hpp"
#include "all_modes.hpp"

#include "log.hpp"

namespace LED
{

  hw_timer_t *loop_led_timer = NULL;

  uint32_t CurrentDuty = 0;

  volatile bool timerFired = false;

  void IRAM_ATTR onTimer()
  {
    timerFired = true;
    if (current_mode)
    {
      current_mode->timer_interrupt();
    }
  }

  void init_led_timer(uint16_t prescaler)
  {
    if (loop_led_timer != NULL)
    {
      timerDetachInterrupt(loop_led_timer);
      timerEnd(loop_led_timer);
      loop_led_timer = NULL;
    }
    loop_led_timer = timerBegin(3, prescaler, true);
    timerAttachInterrupt(loop_led_timer, &LED::onTimer, true);

    stopTimer();
  }

  // timer ticks with 2000 Hz clock so 2000 ticks equals 1 seconds
  void setTimerTime(uint64_t ticks)
  {
    timerAlarmWrite(loop_led_timer, ticks, true);
    timerAlarmEnable(loop_led_timer);
  }

  uint64_t readTimer()
  {
    return timerRead(loop_led_timer);
  }

  void stopTimer()
  {
    timerFired = false;
    timerStop(loop_led_timer);
    timerRestart(loop_led_timer);
  }

  void startTimer()
  {
    timerStart(loop_led_timer);
  }

  void init_led(const uint32_t &freq)
  {
    // attach the channel to the GPIO to be controlled
    int led_id = 0;
    for (const int &led : LED_CHANNELS)
    {
      ledcSetup(led_id, freq, LED_RESOLUTION);
      ledcAttachPin(led, led_id);

      led_id++;
    }
  }

  void set_freq(const uint16_t &channel, const uint32_t &freq)
  {
    if (channel >= LED_CHANNELS_SIZE)
    {
      LOGE("Channel id out of bounds:  %u", channel);

      return;
    }

    ledcChangeFrequency(channel, freq, LED_RESOLUTION);
  }

  void set_duty(const uint16_t &channel, const uint32_t &duty)
  {

    if (channel >= LED_CHANNELS_SIZE)
    {
      LOGE("Channel id out of bounds:  %u", channel);

      return;
    }

    ledcWrite(channel, (duty <= LED_MAX_DUTY) * duty + (duty > LED_MAX_DUTY) * LED_MAX_DUTY);
  }

  void set_freq_all(const uint32_t &freq)
  {
    uint16_t led_id = 0;
    for (const int &led : LED_CHANNELS)
    {
      set_freq(led_id++, freq);
    }
  }

  void set_duty_all(const uint32_t &duty)
  {

    uint16_t led_id = 0;
    for (const int &led : LED_CHANNELS)
    {
      set_duty(led_id++, (duty <= LED_MAX_DUTY) * duty + (duty > LED_MAX_DUTY) * LED_MAX_DUTY);
    }
  }

  uint32_t read_duty(const uint16_t &channel)
  {
    if (channel >= LED_CHANNELS_SIZE)
    {
      LOGE("Channel id out of bounds:  %u", channel);

      return 0;
    }

    return ledcRead(channel);
  }

  uint32_t read_freq(const uint16_t &channel)
  {
    if (channel >= LED_CHANNELS_SIZE)
    {
      LOGE("Channel id out of bounds:  %u", channel);

      return 0;
    }

    return ledcReadFreq(channel);
  }

}