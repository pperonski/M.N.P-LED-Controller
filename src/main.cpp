#include <Arduino.h>
#include <WiFi.h>

#include <LittleFS.h>

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <cmath>

#include <wifimanager/WifiTask.h>
#include <wifimanager/TextInput.h>

#include "led_msg.hpp"
#include "timer_led.hpp"
#include "led_control.hpp"
#include "analog_button.hpp"
#include "configLoad.hpp"
#include "all_modes.hpp"
#include "config.hpp"
#include "network_creditentials.hpp"

#include "log.hpp"

WiFiTask manager(AP_SSID);

volatile bool saveConfig = false;

#ifdef DEBUG

volatile bool NoBatMSGShowed = false;

#endif

mode::Mode *current_mode = NULL;

// when battery voltage is too low, enter deep sleep mode to slow down battery depletion
void checkBatVoltage()
{
  uint16_t vbat = analogRead(V_BAT_PIN);

  if (vbat <= LOW_BAT_VOLTAGE_THRESHOLD)
  {
    LOGW("Low battery voltage going into deep sleep mode...");
    LOGW("Readed value: %u", vbat);

    esp_deep_sleep_start();
  }
}

void saveConfigCallback()
{
  saveConfig = true;
}

TextInput serverURL("serverURL", "Server url", "", 255);

TextInput ipAddr("ip", "IP address", "", 16);

TextInput ipGateway("gateway", "Gateway IP address", "", 16);

TextInput ipMask("mask", "IP mask", "255.255.255.0", 16);

TextInput ipDNS("dns", "DNS IP address", "1.1.1.1", 16);

void loadConfig()
{
  char ip[17] = {0};
  char mask[17] = {0};
  char gateway[17] = {0};
  char dns[17] = {0};

  if (config::loadNetworkConfig(ip, mask, gateway, dns))
  {
    ipAddr.update(ip);
    ipMask.update(mask);
    ipGateway.update(gateway);
    ipDNS.update(dns);

    if (strlen(ip) && strlen(mask) && strlen(gateway))
    {
      LOGI("Setting static ip configuration");
      IPAddress _ip;
      _ip.fromString(ip);
      IPAddress _mask;
      _mask.fromString(mask);
      IPAddress _gateway;
      _gateway.fromString(gateway);
      IPAddress _dns;
      _dns.fromString(dns);

      WiFi.config(_ip, _gateway, _mask, _dns);
      LOGI("IP: %s", ip);
      LOGI("Mask: %s", mask);
      LOGI("Gateway: %s", gateway);
      if (strlen(dns))
      {
        LOGI("DNS: %s", dns);
      }
    }
  }
  else
  {
    strcpy(mask, "255.255.255.0");
    LOGI("No network configuration file, setting default network settings!");

    config::saveNetworkConfig(ip, mask, gateway, dns);
  }
}

void onDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  setLedTimerTime(LED_WIFI_CONNECTING_MSG);
  startLedTimer();
}

void onConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  stopLedTimer();
}

// current mode iterator
volatile uint8_t mode_iterator = 0;

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(USER_LED, OUTPUT);
  digitalWrite(USER_LED, HIGH);

  checkBatVoltage();

  initLedTimer();

  uint8_t fs_connect_attempts = 0;

  setLedTimerTime(FLASH_INIT_FAILED);
  startLedTimer();

  while (!LittleFS.begin(true))
  {

    fs_connect_attempts++;

    // kiedy nie uda się 5 razem zapętl sygnał o błędzie.
    do
    {
      LOGE("Failed to create littleFS partition!");
      delay(500);
    } while (fs_connect_attempts == 5);
  }

  loadConfig();

  // Czy mamy używać SSL czy nie?
  uint8_t *cert = config::loadSSLFile();

  if (cert)
  {
    LOGI("Using secure protocol");
  }
  else
  {
    LOGI("Using non-secure protocol");
  }

  manager.addSaveCallback(saveConfigCallback);

  manager.addParameter(&serverURL);
  manager.addParameter(&ipAddr);
  manager.addParameter(&ipGateway);
  manager.addParameter(&ipDNS);

  manager.begin("wifi", ARDUINO_LOOP_STACK_SIZE, 0);

  WiFi.onEvent(onDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.onEvent(onConnected, ARDUINO_EVENT_WIFI_STA_GOT_IP);

  setLedTimerTime(LED_WIFI_CONNECTING_MSG);
  startLedTimer();

  LOGI("Starting Access Point");

  LED::init_led(LED_FREQUENCY);

  LED::CurrentDuty = LED_MAX_DUTY / 2;

  LED::set_duty_all(LED::CurrentDuty);

  analogButton::initAnalogButton();

  digitalWrite(USER_LED, LOW);

  current_mode = modes_list[mode_iterator]();

  current_mode->setup();
}

void onAnalogButtonState()
{

  switch (analogButton::AnalogButtonCurrentState)
  {
  case BUTTON1 | BUTTON4:
    // change dimm minus one step
    LED::CurrentDuty -= LED_DIMM_STEP;
    LOGD("Current LED duty: %d", LED::CurrentDuty);

    break;

  case BUTTON1:
    // change dimm plus one step
    LED::CurrentDuty += LED_DIMM_STEP;
    LOGD("Current LED duty: %d", LED::CurrentDuty);

    break;

  case BUTTON4:
    // change mode one step forward

    if (current_mode != NULL)
    {
      delete current_mode;
    }

    mode_iterator++;

    if (mode_iterator == MODES_LIST_SIZE)
    {
      mode_iterator = 0;
    }

    current_mode = modes_list[mode_iterator]();

    current_mode->setup();

    break;

  case BUTTON4 | BUTTON3:
    // change mode one step backward

    if (current_mode != NULL)
    {
      delete current_mode;
    }

    mode_iterator--;

    if (mode_iterator > MODES_LIST_SIZE)
    {
      mode_iterator = 0;
    }

    current_mode = modes_list[mode_iterator]();

    current_mode->setup();

    break;
  }

  if (LED::CurrentDuty > 4095)
  {
    LED::CurrentDuty = 0;
  }
}

void loop()
{

  current_mode->loop();

  if (saveConfig)
  {

    char url[1024] = {0};

    char ip[17] = {0};

    char mask[17] = {0};

    char gateway[17] = {0};

    char dns[17] = {0};

    serverURL.getValue(url);
    ipAddr.getValue(ip);
    ipMask.getValue(mask);
    ipGateway.getValue(gateway);
    ipDNS.getValue(dns);

    config::saveServerURL(url);
    config::saveNetworkConfig(ip, mask, gateway, dns);

    saveConfig = false;
  }

  if (analogButton::readAnalogButtonState())
  {
    onAnalogButtonState();

    current_mode->on_button_input(analogButton::AnalogButtonCurrentState);

    analogButton::LastAnalogButtonCurrentState = analogButton::AnalogButtonCurrentState;
  }
}