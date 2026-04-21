#include "modes/Pulse.hpp"
#include "led_control.hpp"

#include "config.hpp"
#include "analog_button.hpp"
#include "log.hpp"

#define OMEGAS_COUNT 100000

namespace mode
{
    void Pulse::setup()
    {
        LOGI("Pulse mode starting...");

        LED::set_duty_all(LED::CurrentDuty);

        LED::init_led_timer();

        LED::startTimer();

        current_mode = ALL;

        oneline_itr = 0;

        pulse_iterator = 0;
    }

    void Pulse::on_button_input(uint8_t state)
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

                LOGD("Pulse: mode one line");

                break;

            case ONELINE:

                oneline_itr++;

                if (oneline_itr == LED_CHANNELS_SIZE)
                {
                    oneline_itr = 0;
                    current_mode = ODDLINES;

                    LOGD("Pulse: mode odd lines");
                }

                break;

            case ODDLINES:

                current_mode = EVENLINES;

                LOGD("Pulse: mode even lines");

                break;

            case EVENLINES:

                current_mode = ALL;

                LOGD("Pulse: mode All");

                break;
            }

            break;

        case BUTTON3:

            LED::stopTimer();

            if (pulse_iterator == OMEGAS_COUNT)
            {
                pulse_iterator = 0;
                LOGD("Stoping pulse mode");
            }
            else
            {
                LED::startTimer();
                LOGD("Starting pulse mode with pulsation: %f Hz", pulse_iterator * 0.1);
                pulse_iterator++;
            }

            break;
        }
    }

    void Pulse::loop()
    {
        LED::set_duty_all(0);

        uint32_t duty = static_cast<uint32_t>(LED::CurrentDuty * ((sin(static_cast<float>(pulse_iterator) * 0.1f * (LED::readTimer() / 2000.f))) + 1.0) / 2.0);

        switch (current_mode)
        {

        case ALL:

            LED::set_duty_all(duty);

            break;

        case ONELINE:

            LED::set_duty(oneline_itr, duty);

            break;

        case ODDLINES:

            for (uint8_t i = 1; i < LED_CHANNELS_SIZE; i += 2)
            {
                LED::set_duty(i, duty);
            }

            break;

        case EVENLINES:

            for (uint8_t i = 0; i < LED_CHANNELS_SIZE; i += 2)
            {
                LED::set_duty(i, duty);
            }

            break;
        }
    }
}