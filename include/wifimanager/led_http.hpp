#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

namespace Ledhttp
{

    void init(AsyncWebServer *server);

    void play(AsyncWebServerRequest *request);

    void setfreq(AsyncWebServerRequest *request);

    void sliderget(AsyncWebServerRequest *request);

    void slider(AsyncWebServerRequest *request);

}