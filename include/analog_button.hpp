#pragma once

#include <Arduino.h>

#define BUTTON1 (1 << 0)
#define BUTTON2 (1 << 1)
#define BUTTON3 (1 << 2)
#define BUTTON4 (1 << 3)

namespace analogButton
{

  extern hw_timer_t *analog_button_timer;

  extern volatile bool AnalogtimerFired;

  extern volatile uint8_t AnalogButtonCurrentState;

  extern volatile uint8_t LastAnalogButtonCurrentState;

  void IRAM_ATTR onAnalogTimer();

  void initAnalogButton();

  /*
    A function that read analog button state.
    Each bit in returned value corresponds to
    diffrent buttons.
  */
  bool readAnalogButtonState();

}