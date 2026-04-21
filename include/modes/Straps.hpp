#pragma once

#include "Mode.hpp"

#include "led_control.hpp"

namespace mode
{

    class Straps : public Mode
    {
        uint16_t blink_iterator;
        uint16_t channel_iterator;

        bool Lights;

    public:
        Straps()
        {
        }

        void on_button_input(uint8_t state) override;

        void setup() override;

        void loop() override;

        ~Straps()
        {
            LED::stopTimer();
        }
    };

}