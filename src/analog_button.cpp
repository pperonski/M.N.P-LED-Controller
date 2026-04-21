#include <cmath>

#include "config.hpp"
#include "analog_button.hpp"

#include "log.hpp"

namespace analogButton
{

    hw_timer_t *analog_button_timer = NULL;

    volatile bool AnalogtimerFired = false;

    volatile uint8_t AnalogButtonCurrentState = 0x00;

    volatile uint8_t LastAnalogButtonCurrentState = 0x00;

    void IRAM_ATTR onAnalogTimer()
    {
        AnalogtimerFired = true;
    }

    void initAnalogButton()
    {
        analog_button_timer = timerBegin(1, 80, true);
        timerAttachInterrupt(analog_button_timer, &onAnalogTimer, true);
        // fire each 1000 ms
        timerAlarmWrite(analog_button_timer, 1000000, true);
        timerAlarmEnable(analog_button_timer);
    }

    bool readAnalogButtonState()
    {
        if (!AnalogtimerFired)
        {
            return false;
        }

        uint16_t abutton = analogRead(ANALOG_BUTTON_PIN) + ANALOG_BUTTON_CALIBR;

        float x = log10(abutton);

        AnalogButtonCurrentState = ceil(pow10(-1.275 * x + 4.3647));

        AnalogButtonCurrentState -= static_cast<uint8_t>(x / 3.25);

        AnalogtimerFired = false;

        return true;
    }

}