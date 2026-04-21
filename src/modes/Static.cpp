#include "modes/Static.hpp"
#include "led_control.hpp"

#include "config.hpp"
#include "analog_button.hpp"
#include "log.hpp"

namespace mode
{
    void Static::setup()
    {
        LOGI("Static mode starting...");

        LED::set_duty_all(LED::CurrentDuty);

        LED::init_led_timer();

        LED::stopTimer();

        current_mode = ALL;

        oneline_itr = 0;

        blink_iterator = 0;

        Lights = true;
    }

    void Static::on_button_input(uint8_t state)
    {
        if (analogButton::AnalogButtonCurrentState == analogButton::LastAnalogButtonCurrentState)
        {
            return;
        }

        switch (state)
        {
        case BUTTON2:

            switch (current_mode)
            {
            case ALL:

                current_mode = ONELINE;

                LOGD("Static: mode one line");

                break;

            case ONELINE:

                oneline_itr++;

                if (oneline_itr == LED_CHANNELS_SIZE)
                {
                    oneline_itr = 0;
                    current_mode = ODDLINES;

                    LOGD("Static: mode odd lines");
                }

                break;

            case ODDLINES:

                current_mode = EVENLINES;

                LOGD("Static: mode even lines");

                break;

            case EVENLINES:

                current_mode = ALL;

                LOGD("Static: mode All");

                break;
            }

            break;

        case BUTTON3:

            LED::stopTimer();

            if (blink_iterator == LED_BLINK_MODES_SIZE)
            {
                blink_iterator = 0;
                Lights = true;
                LOGD("Stoping blink mode");
            }
            else
            {
                LED::setTimerTime(LED_BLINK_MODES[blink_iterator] * 2);
                LED::startTimer();
                LOGD("Starting blink mode with time: %d ms", LED_BLINK_MODES[blink_iterator]);
                blink_iterator++;
            }

            break;
        }
    }

    void Static::loop()
    {
        LED::set_duty_all(0);

        if (LED::timerFired)
        {
            Lights = !Lights;

            LED::timerFired = false;
        }

        if (!Lights)
        {
            return;
        }

        switch (current_mode)
        {

        case ALL:

            LED::set_duty_all(LED::CurrentDuty);

            break;

        case ONELINE:

            LED::set_duty(oneline_itr, LED::CurrentDuty);

            break;

        case ODDLINES:

            for (uint8_t i = 1; i < LED_CHANNELS_SIZE; i += 2)
            {
                LED::set_duty(i, LED::CurrentDuty);
            }

            break;

        case EVENLINES:

            for (uint8_t i = 0; i < LED_CHANNELS_SIZE; i += 2)
            {
                LED::set_duty(i, LED::CurrentDuty);
            }

            break;
        }
    }
}