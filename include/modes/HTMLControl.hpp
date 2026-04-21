#pragma once

#include <Arduino.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Mode.hpp"

#include "log.hpp"

namespace mode
{

    class HTML : public mode::Mode
    {
        // every channel blink frequency
        uint32_t freq[13];

        uint32_t duty[13];

        uint16_t blink_mask;

        uint32_t counter;

    public:
        void timer_interrupt() override;

        static bool HTMLLedControl;

        HTML()
        {
        }

        void setFrequency(uint8_t id, float _freq)
        {
            if (id < 13)
            {
                freq[id] = _freq != 0.0 ? 2000.0 / _freq : 0;

                LOGD("HTML LED freq: %d value %d", id, freq[id]);
            }
        }

        void setDuty(uint8_t id, float val)
        {
            if (id < 13)
            {
                duty[id] = (val / 100.0) * 4095;

                LOGD("HTML LED id: %d value %d", id, duty[id]);
            }
        }

        void setup();

        void loop();

        ~HTML();
    };

}