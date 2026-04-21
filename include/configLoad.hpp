#pragma once

#include <Arduino.h>

#include <ArduinoJson.h>

namespace config
{

    bool loadServerURL(char *url);

    bool saveServerURL(const char *url);

    bool loadNetworkConfig(char *ip, char *mask, char *gateway, char *dns);

    bool saveNetworkConfig(const char *ip, const char *mask, const char *gateway, const char *dns);

    uint8_t *loadSSLFile();

}