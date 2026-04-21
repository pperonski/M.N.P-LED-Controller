/*

    An abstract class for led driving mode.

    Modes to implement:

    Static light.
    Blinking.
    02API driving. - use one timer and modulo to blink all channels
    HttpServer control.
*/

#pragma once

#include <Arduino.h>

namespace mode
{

    class Mode
    {
    protected:
    public:
        virtual void timer_interrupt() {}

        Mode()
        {
        }

        virtual void on_button_input(uint8_t state)
        {
        }

        virtual void setup() = 0;

        /*
            Main loop function, as argument is going to get input from analog button.
        */
        virtual void loop() = 0;

        virtual ~Mode()
        {
        }
    };

}