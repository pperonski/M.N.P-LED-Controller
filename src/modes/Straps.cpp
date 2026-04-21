#include "modes/Straps.hpp"
#include "led_control.hpp"

#include "config.hpp"
#include "analog_button.hpp"
#include "log.hpp"

namespace mode
{
    void Straps::setup()
    {
        LOGI("Straps mode starting...");

        LED::set_duty_all(LED::CurrentDuty);

        LED::init_led_timer();

        LED::stopTimer();

        blink_iterator = 0;
        channel_iterator = 0;

        Lights = true;

        LED::setTimerTime(LED_BLINK_MODES[blink_iterator] * 2);
        LED::startTimer();
    }

    void Straps::on_button_input(uint8_t state)
    {
        if (analogButton::AnalogButtonCurrentState == analogButton::LastAnalogButtonCurrentState)
        {
            return;
        }

        switch (state)
        {
        case BUTTON2:

            Lights != Lights;

        break;
        case BUTTON3:

            LED::stopTimer();

            LED::setTimerTime(LED_BLINK_MODES[blink_iterator] * 2);
            LED::startTimer();
            LOGD("Starting blink mode with time: %d ms", LED_BLINK_MODES[blink_iterator]);
            blink_iterator++;

            if( blink_iterator == LED_BLINK_MODES_SIZE )
            {
                blink_iterator = 0;
            }

            break;
        }
    }

    void Straps::loop()
    {
        LED::set_duty_all(0);

        if (LED::timerFired)
        {
            Lights = !Lights;

            if(!Lights)
            {
                channel_iterator ++;
                channel_iterator = channel_iterator % 3;
            }

            LED::timerFired = false;
        }

        if (!Lights)
        {
            return;
        }

        for(uint8_t i=0;i<LED_CHANNELS_SIZE;++i)
        {
            if( ( i % channel_iterator ) == 0 )
            {
                LED::set_duty(i,LED::CurrentDuty);
            }
        }

    }
}