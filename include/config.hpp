#pragma once

#include <cstdint>

// Replace with your network credentials
#define AP_SSID "Przejście dla pieszych"

#define NETWORK_CONFIG_FILENAME "/network.json"

#define SERVER_CONFIG_FILENAME "/server.json"

#define SSL_CERT_FILENAME "/cert"

// led pins
extern const uint16_t LED_CHANNELS[13];

#define LED_CHANNELS_SIZE sizeof(LED_CHANNELS) / sizeof(uint16_t)

#define USER_LED 23

// a battery voltage value below MCU goes to deep sleep
#define LOW_BAT_VOLTAGE_THRESHOLD 2865

// if battery volatage is very low or near 0 it means that battery is not connected at all
#define NO_BAT_CONNECTED_THRESHOLD 1650

#define V_BAT_PIN 39

#define ANALOG_BUTTON_PIN 36

#define ANALOG_BUTTON_CALIBR 192

extern const char *PARAM_INPUT;

#define LED_RESOLUTION 12

#define LED_MAX_DUTY 4095

#define LED_FREQUENCY 10000

#define LED_DIMM_STEP 40

extern const uint32_t LED_BLINK_MODES[6];

#define LED_BLINK_MODES_SIZE sizeof(LED_BLINK_MODES) / sizeof(uint32_t)