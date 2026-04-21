#include "modes/APIControl.hpp"
#include <cstring>
#include "log.hpp"

namespace mode
{

    void API::on_reciv(mqtt::MQTT *mqtt, const char *topic, const char *data, int data_len, int msg_id)
    {

        if (strcmp(topic, MQTT_MPN_TOPIC) != 0)
        {
            return;
        }

        // do something with message

        StaticJsonDocument<1024> json;

        deserializeJson(json, data, data_len);

        if (json.containsKey("frequency"))
        {
            JsonArray freqs = json["frequency"];

            uint8_t iter = 0;

            for (auto f : freqs)
            {
                // timer ticks with 2kHz clock
                float _f = f;
                this->freq[iter++] = 2000.0 / _f;
            }

            for (uint8_t i = iter; i < LED_CHANNELS_SIZE; ++i)
            {
                this->freq[i] = 0;
            }
        }

        if (json.containsKey("brightness"))
        {
            JsonArray brightness = json["brightness"];

            uint8_t iter = 0;

            for (auto brigh : brightness)
            {
                this->duty[iter++] = brigh;
            }

            for (uint8_t i = iter; i < LED_CHANNELS_SIZE; ++i)
            {
                this->duty[iter++] = 0;
            }
        }

        this->api.subscribe(MQTT_MPN_TOPIC);
    }

    void API::on_connected(mqtt::MQTT *mqtt)
    {
        // subscribe and wait for messages

        this->api.subscribe(MQTT_MPN_TOPIC);
    }

    void API::timer_interrupt()
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

    void API::setup()
    {
        LOGI("Prepearing API control");

        char url[1024] = {0};

        if (config::loadServerURL(url) && strlen(url))
        {

            api.setUserData(MQTT_USERNAME, MQTT_PASSWORD);

            api.setOnReciv([this](mqtt::MQTT *mqtt, const char *topic, const char *data, int data_len, int msg_id)
                           { this->on_reciv(mqtt, topic, data, data_len, msg_id); });

            api.setOnConnect([this](mqtt::MQTT *mqtt)
                             { this->on_connected(mqtt); });

            if (api.begin(url, (const char *)creditentials::cert))
            {
                LOGI("Connected to a server: %s", url);
            }
            else
            {
                LOGE("Cannot connect to server!!");
            }
        }
        else
        {
            LOGE("No server url setted up!!");
        }

        LED::init_led_timer(400);

        LED::stopTimer();

        LED::setTimerTime(10);

        LED::startTimer();
    }

    void API::loop()
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

            ++iter;
        }
    }

}