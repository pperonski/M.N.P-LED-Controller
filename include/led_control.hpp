/*

    A header that contain function for led control.

*/
#pragma once

#include <Arduino.h>
#include <functional>

#include "config.hpp"

namespace LED
{

    extern hw_timer_t *loop_led_timer;

    extern uint32_t CurrentDuty;

    extern volatile bool timerFired;

    void IRAM_ATTR onTimer();

    void init_led_timer(uint16_t prescaler = 40000);

    void setTimerTime(uint64_t ticks);

    uint64_t readTimer();

    void stopTimer();

    void startTimer();

    void init_led(const uint32_t &freq);

    void init_led_timer_with_interrupt(uint16_t prescaler, void (*callback)());

    void set_freq(const uint16_t &channel, const uint32_t &freq);

    void set_duty(const uint16_t &channel, const uint32_t &duty);

    void set_freq_all(const uint32_t &freq);

    void set_duty_all(const uint32_t &duty);

    uint32_t read_duty(const uint16_t &channel);

    uint32_t read_freq(const uint16_t &channel);

} // namespace LED