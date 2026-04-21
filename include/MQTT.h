#pragma once

#include <functional>

/*
 A simple class for MQTT connection handling.

*/

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "mqtt_client.h"

namespace mqtt
{

    class MQTT;

    typedef std::function<void(MQTT *)> on_connect_call;

    typedef std::function<void(MQTT *)> on_disconnected_call;

    // mqtt instance, topic , msg_id
    typedef std::function<void(MQTT *, const char *, int)> on_subscribed_call;

    // mqtt instance, topic , msg_id
    typedef std::function<void(MQTT *, const char *, int)> on_unsubscribed_call;

    // mqtt instance, topic , msg_id
    typedef std::function<void(MQTT *, const char *, int)> on_published_call;

    // mqtt instance, topic , data, data size , msg_id
    typedef std::function<void(MQTT *, const char *, const char *, int, int)> on_recive_call;

    // mqtt instance, topic ,msg_id, error_type
    typedef std::function<void(MQTT *, const char *, int, int)> on_error_call;

    struct mqtt_callbacks
    {

        MQTT *_mqtt;

        // mqtt instance
        on_connect_call on_connected;

        // mqtt instance
        on_disconnected_call on_disconnected;

        // mqtt instance, topic , msg_id
        on_subscribed_call on_subscribed;

        // mqtt instance, topic , msg_id
        on_unsubscribed_call on_unsubscribed;

        // mqtt instance, topic , msg_id
        on_published_call on_published;

        // mqtt instance, topic , data, data size , msg_id
        on_recive_call on_recive;

        // mqtt instance, topic ,msg_id, error_type
        on_error_call on_error;
    };

    class MQTT
    {
    protected:
        static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
        {
            esp_mqtt_client_handle_t client = event->client;

            mqtt_callbacks *calls = (mqtt_callbacks *)(event->user_context);

            int msg_id;
            // your_context_t *context = event->context;
            switch (event->event_id)
            {
            case MQTT_EVENT_CONNECTED:

                if (calls->on_connected)
                {
                    calls->on_connected(calls->_mqtt);
                }

                break;
            case MQTT_EVENT_DISCONNECTED:

                if (calls->on_disconnected)
                {
                    calls->on_disconnected(calls->_mqtt);
                }

                break;

            case MQTT_EVENT_SUBSCRIBED:

                if (calls->on_subscribed)
                {
                    calls->on_subscribed(calls->_mqtt, event->topic, event->msg_id);
                }

                break;
            case MQTT_EVENT_UNSUBSCRIBED:

                if (calls->on_unsubscribed)
                {
                    calls->on_unsubscribed(calls->_mqtt, event->topic, event->msg_id);
                }

                break;
            case MQTT_EVENT_PUBLISHED:

                if (calls->on_published)
                {
                    calls->on_published(calls->_mqtt, event->topic, event->msg_id);
                }

                break;
            case MQTT_EVENT_DATA:
                // ważne
                if (calls->on_recive)
                {
                    calls->on_recive(calls->_mqtt, event->topic, event->data, event->data_len, event->msg_id);
                }

                break;
            case MQTT_EVENT_ERROR:

                if (calls->on_error)
                {

                    calls->on_error(calls->_mqtt, event->topic, event->msg_id, event->error_handle->error_type);
                }

                break;
            }
            return ESP_OK;
        }

        static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
        {
            /* The argument passed to esp_mqtt_client_register_event can de accessed as handler_args*/
            mqtt_event_handler_cb((esp_mqtt_event_handle_t)event_data);
        }

        esp_mqtt_client_config_t mqtt_cfg;

        esp_mqtt_client_handle_t client;

        mqtt_callbacks callbacks;

    public:
        MQTT()
        {
            mqtt_cfg = {0};

            callbacks._mqtt = this;

            mqtt_cfg.user_context = &callbacks;

            client = nullptr;

            uint64_t _chipmacid = 0;
            esp_efuse_mac_get_default((uint8_t *)(&_chipmacid));

            setId(_chipmacid);
        }

        void setUserData(const char *user, const char *pass)
        {
            setUsername(user);
            setPassword(pass);
        }

        void setUserData(int id, const char *user, const char *pass)
        {
            setUsername(user);
            setPassword(pass);
            setId(id);
        }

        void setUserData(const char *id, const char *user, const char *pass)
        {
            setUsername(user);
            setPassword(pass);
            setId(id);
        }

        void setId(const char *id)
        {
            mqtt_cfg.client_id = id;
        }

        void setId(uint64_t id)
        {
            if (mqtt_cfg.client_id != NULL)
            {
                delete mqtt_cfg.client_id;
            }

            mqtt_cfg.client_id = new char[64];

            itoa(id, (char *)mqtt_cfg.client_id, 64);
        }

        void setUsername(const char *user)
        {
            mqtt_cfg.username = user;
        }

        void setPassword(const char *pass)
        {
            mqtt_cfg.password = pass;
        }

        void setCert(const char *cert)
        {
            mqtt_cfg.cert_pem = cert;
        }

        void setAddress(const char *uri)
        {
            mqtt_cfg.uri = uri;
        }

        void setPort(int port)
        {
            mqtt_cfg.port = port;
        }

        void setCallbacks(mqtt_callbacks calls)
        {
            callbacks = calls;
            callbacks._mqtt = this;
        }

        void setOnConnect(on_connect_call call)
        {
            callbacks.on_connected = call;
        }

        void setOnDisconnect(on_disconnected_call call)
        {
            callbacks.on_disconnected = call;
        }

        void setOnSubscribe(on_subscribed_call call)
        {
            callbacks.on_subscribed = call;
        }

        void setOnUnsubscribe(on_unsubscribed_call call)
        {
            callbacks.on_unsubscribed = call;
        }

        void setOnPublish(on_published_call call)
        {
            callbacks.on_published = call;
        }

        void setOnReciv(on_recive_call call)
        {
            callbacks.on_recive = call;
        }

        void setOnError(on_error_call call)
        {
            callbacks.on_error = call;
        }

        bool begin()
        {
            client = esp_mqtt_client_init(&mqtt_cfg);

            if (client != NULL)
            {

                esp_mqtt_client_register_event(client, (esp_mqtt_event_id_t)ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);

                return esp_mqtt_client_start(client) == ESP_OK;
            }

            return false;
        }

        bool begin(const char *address, const char *cert = nullptr)
        {
            setAddress(address);
            setCert(cert);

            return begin();
        }

        bool begin(const char *address, int port, const char *cert = nullptr)
        {
            setAddress(address);
            setPort(port);
            setCert(cert);

            return begin();
        }

        int publish(const char *topic, const char *data, int size)
        {
            return esp_mqtt_client_publish(client, topic, data, size, 0, 0);
        }

        int publish(const char *topic, const char *data)
        {
            int size = strlen(data);

            return esp_mqtt_client_publish(client, topic, data, size, 0, 0);
        }

        int subscribe(const char *topic)
        {
            return esp_mqtt_client_subscribe(client, topic, 0);
        }

        int unsubscribe(const char *topic)
        {
            return esp_mqtt_client_unsubscribe(client, topic);
        }

        void start()
        {
            esp_mqtt_client_start(client);
        }

        void disconnect()
        {
            esp_mqtt_client_disconnect(client);
        }

        void end()
        {
            esp_mqtt_client_stop(client);
        }

        ~MQTT()
        {
            if (mqtt_cfg.client_id != NULL)
            {
                delete mqtt_cfg.client_id;
            }
            disconnect();
            end();
        }
    };

}