/*
    It accepts json messages onto MQTT_MPN_TOPIC like:

    {
        frequency:[f1,f2,f3...f13],
        brightness:[duty1,duty2,...,duty13]
    }

*/

#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Mode.hpp"
#include "MQTT.h"

#include "configLoad.hpp"

#include "network_creditentials.hpp"

#include "led_control.hpp"

using mqtt::MQTT;

#define MQTT_USERNAME "overlord"

#define MQTT_PASSWORD "pIlY-PPeLzs1enKRAiAjWCd9GOghZgR80HOFcbwHAyo"

#define MQTT_MPN_TOPIC "/MPN/api"

namespace mode
{

    class API : public mode::Mode
    {
        MQTT api;

        // every channel blink frequency
        uint32_t freq[13];

        uint32_t duty[13];

        uint16_t blink_mask;

        void on_reciv(mqtt::MQTT *mqtt, const char *topic, const char *data, int data_len, int msg_id);

        void on_connected(mqtt::MQTT *mqtt);

        uint64_t counter;

    public:
        void timer_interrupt() override;

        API()
        {
            blink_mask = 0x00;
        }

        void setup();

        void loop();

        ~API()
        {
        }
    };

}