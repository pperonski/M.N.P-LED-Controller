#include "modes/HTMLControl.hpp"

#include "led_control.hpp"
#include "log.hpp"

namespace mode
{

    bool HTML::HTMLLedControl = false;

    void HTML::timer_interrupt()
    {
        counter++;

        uint8_t iter = 0;

        for (const uint32_t &f : this->freq)
        {
            if (f != 0)
            {
                if (counter % f == 0)
                {
                    this->blink_mask ^= (1 << iter);
                }
            }
            else
            {
                this->blink_mask |= (1 << iter);
            }

            ++iter;
        }
    }

    void HTML::setup()
    {

        LOGI("HTML LED control page mode starting...");

        LED::set_duty_all(2048);

        LED::init_led_timer(400);

        LED::stopTimer();

        LED::setTimerTime(10);

        LED::startTimer();

        HTMLLedControl = true;
    }

    void HTML::loop()
    {
        uint8_t iter = 0;

        for (const uint32_t &f : this->freq)
        {

            if (this->blink_mask & (1 << iter))
            {
                LED::set_duty(iter, this->duty[iter]);
            }
            else
            {
                LED::set_duty(iter, 0);
            }

            iter++;
        }
    }

    HTML::~HTML()
    {
        HTMLLedControl = false;
    }

}