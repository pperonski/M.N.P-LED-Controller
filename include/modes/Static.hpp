#pragma once

#include "Mode.hpp"

#include "led_control.hpp"

namespace mode
{

    class Static : public Mode
    {

        enum StaticMode
        {
            ALL,
            ONELINE,
            ODDLINES,
            EVENLINES
        };

        uint8_t oneline_itr;

        StaticMode current_mode;

        uint16_t blink_iterator;

        bool Lights;

    public:
        Static()
        {
        }

        void on_button_input(uint8_t state) override;

        void setup() override;

        void loop() override;

        ~Static()
        {
            LED::stopTimer();
        }
    };

}