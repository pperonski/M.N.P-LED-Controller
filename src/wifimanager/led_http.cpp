#include <Arduino.h>

#include <wifimanager/LedPage.h>
#include <wifimanager/led_http.hpp>

#include "config.hpp"

namespace Ledhttp
{

  void init(AsyncWebServer *server)
  {

    // Route for root / web page
    server->on("/setfreq", HTTP_GET, setfreq);

    server->on("/sliderget", HTTP_GET, sliderget);

    // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
    server->on("/slider", HTTP_GET, slider);
  }

  void setfreq(AsyncWebServerRequest *request)
  {

    int freq = 0;
    if (request->hasParam(PARAM_INPUT))
    {
      int value;
      int led_id = 0;

      value = request->getParam(PARAM_INPUT)->value().toInt();

      for (const int &led : LED_CHANNELS)
      {
        ledcChangeFrequency(led_id, value, 8);

        led_id++;
      }
    }

    request->send(200, "text/plain", "OK");
  }

  void sliderget(AsyncWebServerRequest *request)
  {

    AsyncWebServerResponse *response = request->beginResponse(200);

    int led_id = 0;
    int value = 0;
    for (const int &led : LED_CHANNELS)
    {
      value = ledcRead(led_id);
      response->addHeader(String(led_id), String(value));

      led_id++;
    }

    request->send(response);
  }

  void slider(AsyncWebServerRequest *request)
  {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT))
    {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      int value = inputMessage.toInt();

      int id = request->getParam("id")->value().toInt();
      Serial.print("Current id: ");
      Serial.println(id);

      Serial.print("Current value: ");
      Serial.println(value);

      ledcWrite(id, value);
    }
    else
    {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  }

}
