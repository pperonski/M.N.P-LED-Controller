#ifndef WIFI_TASK_H
#define WIFI_TASK_H

#include <Arduino.h>

#include <string>
#include <vector>
#include <functional>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "Parameter.h"

#define NULL_MSG "None"

#define WIFI_DELAY 4000

#define HTTP_PORT 80

#define SSID_LINE_LENGTH 32

#define WIFI_JSON_BUFFER 2048

#define MAX_SSID 10 // how much ssid to store in flash

#define SECRET_PASSWORD "ra_xviii_7312"

typedef std::function<void()> SaveCallback;

class WiFiTask
{
private:
  // store the html web parameters
  std::vector<Parameter *> params;

  // function to run when server get parameters' data to save
  SaveCallback saveCallback;

  StaticTask_t xTaskBuffer;

  StackType_t *xStack;

  TaskHandle_t xHandle;

  // a http endpoints:

  static void handle404(AsyncWebServerRequest *request);

  // a root html page
  void handleRoot(AsyncWebServerRequest *request);

  // a style css
  void handleCSS(AsyncWebServerRequest *request);

  // a javascript
  void handleJS(AsyncWebServerRequest *request);

  // send list of saved networks
  void listSSID(AsyncWebServerRequest *request);

  // send list of scanned networks
  void scanSSID(AsyncWebServerRequest *request);

  // commit a new ssid scan
  void beginScanSSID(AsyncWebServerRequest *request);

  // get SSID information by specified id by header
  void getSSID(AsyncWebServerRequest *request);

  // try to connect to a network and then register it into a json file, input is ssid and password in header
  void registerSSID(AsyncWebServerRequest *request);

  // change existing network password, by ssid and password in header
  void updateSSID(AsyncWebServerRequest *request);

  // remove network from file, by ssid in header
  void removeSSID(AsyncWebServerRequest *request);

  // remove file with networks
  void clearSSID(AsyncWebServerRequest *request);

  // diconnect from current network
  void disconnectSSID(AsyncWebServerRequest *request);

  // update the parameter with specified id by param , (id -> <0;size-1> )
  void setParameter(AsyncWebServerRequest *request);

  // get the parameter settings with specified id by param , (id -> <0;size-1> ) , value is passed by header
  void readParameter(AsyncWebServerRequest *request);

  // get the number of parameters
  void ParameterNumber(AsyncWebServerRequest *request);

  // send when you are sure that the parameters can be saved
  void UpdateParameter(AsyncWebServerRequest *request);

  void setLedDuty(AsyncWebServerRequest *request);

  void setLedFreq(AsyncWebServerRequest *request);

protected:
  uint32_t wlan_start; // a timer for wifi timeout

  uint32_t WiFiTimeout; // a wlan connection timeout in miliseconds

  bool FindConnection; // perform connection ?

  bool EnterAPMode; // enter AP mode ( acess point mode)

  bool startScanning; // perform SSID scan

  AsyncWebServer *backend;

  const char *ssid_file; // path to file with ssids

  uint32_t to_timeout; // timeout in miliseconds to connect

  char *ssid; // current network ssid
  char *pwd;  // current network password

  char *ap_name; // default ap name

  void onStart();

  void onRemove();

  static void PackSSID(JsonObject &obj, uint32_t id);

  static void PackSSID(JsonObject &obj, const char *ssid);

  // pack connected network data
  static void PackSSID(JsonObject &obj);

  // remove ssid from json, with specified ssid
  bool remove_ssid_json(char *ssid);

  // add ssid to json, with password
  bool add_json_ssid(char *ssid, char *pwd);

  // retrun password assigned to ssid into pwd pointer
  bool get_pwd_from_ssid(char *ssid, char *pwd);

  // checks if there is network with specified ssid in network file
  bool check_file_for_ssid(char *ssid);

  // check if network file exists
  bool check_ssid_file();

  // connect to wifi and iterate through ssids
  bool connect();
  // set to the stationary access point
  bool enter_AP();
  // exit the stationary access point
  bool exit_AP();

  // set when class eneter connect() function
  bool Connecting;

  // set when wlan needs to disconnect
  bool toDisconnect;

  // set when parameters need to be saved
  bool SaveParameters;

  bool isRunning;

public:
  WiFiTask(const char *_ssid, bool _StartWebServer = true);

  void begin(const char *name, const uint32_t stack_size, uint32_t piority);

  static void static_loop(void *arg)
  {
    ((WiFiTask *)arg)->loop();
  }

  void loop();

  // set connection timeout in tries number;
  void setTimeout(uint32_t _timeout);

  // add new parameter, parameter must be created before the function, you cannot pass it by new operator
  void addParameter(Parameter *_param);

  // remove parameters
  void clearParameters();

  // add a function to be called when parameters are saved
  void addSaveCallback(SaveCallback _callback);

  // function to stop web server
  void StopWebServer();

  // function to start web server , on default it runs on begin of the class
  void StartWebServer();

  // return true if web server is running, and false when web server has been stopped
  bool WebState();
};

#endif
