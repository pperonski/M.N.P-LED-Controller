#include <wifimanager/WifiTask.h>

#include <wifimanager/index.h>
#include <wifimanager/style.h>
#include <wifimanager/script.h>
#include <wifimanager/led_http.hpp>
#include <wifimanager/LedPage.h>

#include "all_modes.hpp"

#include "led_control.hpp"

WiFiTask::WiFiTask(const char *_ssid, bool _StartWebServer)
{
  isRunning = _StartWebServer;

  ap_name = (char *)calloc(SSID_LINE_LENGTH, sizeof(char));

  strcpy(ap_name, _ssid);

  ssid = (char *)calloc(SSID_LINE_LENGTH, sizeof(char));

  pwd = (char *)calloc(SSID_LINE_LENGTH, sizeof(char));

  WiFiTimeout = 5000;

  to_timeout = millis();
}

void WiFiTask::begin(const char *name, const uint32_t stack_size, uint32_t piority)
{
  onStart();
  xStack = (StackType_t *)calloc(stack_size, sizeof(StackType_t));
  xHandle = xTaskCreateStatic(
      static_loop, /* Function that implements the task. */
      name,        /* Text name for the task. */
      stack_size,  /* Number of indexes in the xStack array. */
      this,        /* Parameter passed into the task. */
      piority,     /* Priority at which the task is created. */
      xStack,
      &xTaskBuffer);
}

void WiFiTask::loop()
{
  while (true)
  {

    if (startScanning)
    {

      WiFi.scanNetworks();

      startScanning = false;
    }

    if (toDisconnect)
    {
      if (remove_ssid_json(const_cast<char *>(WiFi.SSID().c_str())))
      {
        Serial.println("Wifi has been removed!");
      }

      if (WiFi.disconnect())
      {

        Serial.println("Wifi has been disconnected!");
      }

      toDisconnect = false;
    }

    if (SaveParameters)
    {
      saveCallback();
      SaveParameters = false;
      Serial.println("Parameters has been saved!");
    }

    if (EnterAPMode)
    {
      if (enter_AP())
      {
        Serial.println("Entering the AP mode!");
        EnterAPMode = false;
      }
      else
      {
        Serial.println("Failed to start AP mode!");
      }
    }

    if (FindConnection)
    {

      Connecting = true;

      this->connect();

      Connecting = false;

      if (WiFiTimeout > 0)
      {

        if (millis() - to_timeout >= WiFiTimeout)
        {
          FindConnection = false;
          EnterAPMode = true;

          to_timeout = millis();
        }
        else
        {
          Serial.println("Connection timeout");
        }
      }
    }
  }
}

void WiFiTask::setTimeout(uint32_t _timeout)
{
  WiFiTimeout = _timeout;
}

bool WiFiTask::exit_AP()
{
  return WiFi.softAPdisconnect(true);
}

bool WiFiTask::enter_AP()
{
  return WiFi.softAP(ap_name,SECRET_PASSWORD);
}

bool WiFiTask::connect()
{

  Serial.println("Trying to connect...");

  if (WiFi.SSID().length() > 0)
  {

    if (WiFi.reconnect())
    {
      FindConnection = false;
      EnterAPMode = false;
      Serial.print("Connected to previous network with SSID: ");
      Serial.println(WiFi.SSID());
      return true;
    }
  }

  if (!check_ssid_file())
  {
    EnterAPMode = true;
    FindConnection = false;
    Serial.println("Cannot load ssid list!");
    return false;
  }

  uint16_t wlans = WiFi.scanNetworks();

  Serial.print(wlans);

  Serial.println(" networks has been found!");

  if (wlans == 0)
  {
    EnterAPMode = true;
    FindConnection = false;
    return false;
  }

  for (uint16_t i = 0; i < wlans; i++)
  {
    memset(ssid, 0, SSID_LINE_LENGTH * sizeof(char));
    WiFi.SSID(i).toCharArray(ssid, SSID_LINE_LENGTH);

    if (get_pwd_from_ssid(ssid, pwd))
    {
      Serial.print("Trying to connect with ssid ");
      Serial.println(ssid);
      if (WiFi.begin(ssid, pwd))
      {
        FindConnection = false;
        EnterAPMode = false;
        return true;
      }
    }
  }

  Serial.println("Cannot match scaned ssids with ssids in file!");

  memset(ssid, 0, SSID_LINE_LENGTH * sizeof(char));
  memset(pwd, 0, SSID_LINE_LENGTH * sizeof(char));
  return false;
}

bool WiFiTask::check_ssid_file()
{
  if (!LittleFS.exists("/wlans.json"))
  {
    return false;
  }

  StaticJsonDocument<WIFI_JSON_BUFFER> doc;

  fs::File file = LittleFS.open("/wlans.json", FILE_READ);

  if (!file)
  {
    return false;
  }

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    file.close();
    return false;
  }

  if (doc.size() <= 0)
  {
    return false;
  }

  file.close();
  return true;
}

bool WiFiTask::get_pwd_from_ssid(char *ssid, char *pwd)
{
  fs::File file = LittleFS.open("/wlans.json", FILE_READ);

  if (!file)
  {
    Serial.println("No ssid to read");
    return false;
  }

  StaticJsonDocument<WIFI_JSON_BUFFER> doc;

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println("Failed to read file for ssid removal");
    file.close();
    return false;
  }

  if (doc.containsKey(ssid))
  {
    memset(pwd, 0, SSID_LINE_LENGTH * sizeof(char));
    strcpy(pwd, doc[ssid]);
  }
  else
  {
    file.close();
    return false;
  }

  file.close();
  return true;
}

bool WiFiTask::check_file_for_ssid(char *ssid)
{
  fs::File file = LittleFS.open("/wlans.json", FILE_READ);

  if (!file)
  {
    Serial.println("No ssid to read");
    return false;
  }

  StaticJsonDocument<WIFI_JSON_BUFFER> doc;

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println("Failed to read file for ssid removal");
    file.close();
    return false;
  }

  if (!doc.containsKey(ssid))
  {
    file.close();
    return false;
  }

  file.close();
  return true;
}

bool WiFiTask::add_json_ssid(char *ssid, char *pwd)
{

  fs::File file = LittleFS.open("/wlans.json", FILE_READ);

  StaticJsonDocument<WIFI_JSON_BUFFER> doc;

  if (!file)
  {
    Serial.println("Error reading ssid data from json");
    // emit_event(SSID_WRITE_ERR,NULL);
    // return false;
  }
  else
  {

    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      Serial.println("Failed to read file, for ssid update");
      return false;
    }

    file.close();
  }

  if (doc.size() >= MAX_SSID)
  {
    Serial.println("Too much ssid to save");
    return false;
  }

  file = LittleFS.open("/wlans.json", FILE_WRITE);

  if (!file)
  {
    Serial.println("Error updating ssid data from json");
    return false;
  }

  // doc.add(encapsulateSSID(ssid,pwd));

  doc[ssid] = pwd;

  if (serializeJson(doc, file) == 0)
  {
    Serial.println("Failed to write ssid to file");
    return false;
  }

  return true;
}

bool WiFiTask::remove_ssid_json(char *ssid)
{
  fs::File file = LittleFS.open("/wlans.json", FILE_READ);

  if (!file)
  {
    Serial.println("Error reading ssid data from json for removal");
    return false;
  }

  StaticJsonDocument<WIFI_JSON_BUFFER> doc;

  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println("Failed to read file for ssid removal");
    return false;
  }

  file.close();

  file = LittleFS.open("/wlans.json", FILE_WRITE);

  if (!file)
  {
    Serial.println("Error updating ssid data from json, for removal");
    return false;
  }

  Serial.print("Removing ssid: ");
  Serial.println(ssid);

  doc.remove(ssid);

  if (serializeJson(doc, file) == 0)
  {
    Serial.println("Failed to write ssid to file");
    return false;
  }

  return true;
}

void WiFiTask::PackSSID(JsonObject &obj)
{

  obj["ssid"] = WiFi.SSID();
  obj["rssd"] = WiFi.RSSI();
  obj["mac"] = WiFi.BSSIDstr();
  obj["channel"] = WiFi.channel();
  obj["connected"] = true;
}

void WiFiTask::PackSSID(JsonObject &obj, uint32_t id)
{

  obj["ssid"] = WiFi.SSID(id);
  obj["rssd"] = WiFi.RSSI(id);
  obj["mac"] = WiFi.BSSIDstr(id);
  obj["enc"] = WiFi.encryptionType(id);
  obj["channel"] = WiFi.channel(id);
  obj["connected"] = false;
}

void WiFiTask::PackSSID(JsonObject &obj, const char *ssid)
{

  obj["ssid"] = ssid;
}

void WiFiTask::onRemove()
{
  delete backend;
  free(ssid);
  free(pwd);
  free(ap_name);
}

void WiFiTask::onStart()
{
  Connecting = false;

  startScanning = false;

  FindConnection = true;
  to_timeout = millis();

  toDisconnect = false;

  EnterAPMode = false;

  SaveParameters = false;

  WiFi.mode(WIFI_MODE_STA);

  // start the internal file system
  while (!LittleFS.begin(true))
  {
    Serial.println("Waitting for FLASH memory");

    delay(500);
  }

  backend = new AsyncWebServer(HTTP_PORT);

  backend->on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->handleRoot(request); });

  backend->on("/web/style.css", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->handleCSS(request); });

  backend->on("/web/script.js", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->handleJS(request); });

  backend->on("/led/duty", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->setLedDuty(request); });

  backend->on("/led/freq", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->setLedFreq(request); });

  backend->on("/1234", HTTP_GET, [this](AsyncWebServerRequest *request) -> void
              { this->scanSSID(request); });

  backend->on("/1235", HTTP_GET, [this](AsyncWebServerRequest *request) -> void
              { this->beginScanSSID(request); });

  backend->on("/1243", HTTP_GET, [this](AsyncWebServerRequest *request) -> void
              { this->getSSID(request); });

  backend->on("/1456", HTTP_GET, [this](AsyncWebServerRequest *request) -> void
              { this->listSSID(request); });

  backend->on("/2441", HTTP_POST, [this](AsyncWebServerRequest *request)
              { this->registerSSID(request); });

  backend->on("/2445", HTTP_POST, [this](AsyncWebServerRequest *request)
              { this->updateSSID(request); });

  backend->on("/3561", HTTP_DELETE, [this](AsyncWebServerRequest *request)
              { this->removeSSID(request); });

  backend->on("/3562", HTTP_DELETE, [this](AsyncWebServerRequest *request)
              { this->clearSSID(request); });

  backend->on("/3570", HTTP_DELETE, [this](AsyncWebServerRequest *request)
              { this->disconnectSSID(request); });

  backend->on("/6213", HTTP_POST, [this](AsyncWebServerRequest *request)
              { this->setParameter(request); });

  backend->on("/6244", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->readParameter(request); });

  backend->on("/6236", HTTP_GET, [this](AsyncWebServerRequest *request)
              { this->ParameterNumber(request); });

  backend->on("/6256", HTTP_POST, [this](AsyncWebServerRequest *request)
              { this->UpdateParameter(request); });

  backend->serveStatic("/wlans.json", LittleFS, "/wlans.json");

  backend->onNotFound(handle404);

  Ledhttp::init(backend);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
               {
      Serial.print("WiFi got ip address: ");
      Serial.println(WiFi.localIP()); }, ARDUINO_EVENT_WIFI_STA_GOT_IP);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
               {
                 Serial.print("WiFi connected to ssid: ");
                 Serial.println(WiFi.SSID());
                 to_timeout = 0;
                 exit_AP(); },
               ARDUINO_EVENT_WIFI_STA_CONNECTED);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
               {
      // do nothing when class is tempting to connect to some network
      if(Connecting)
      {
        return;
      }

       FindConnection=true;
       to_timeout=millis();  

       Serial.println("WiFi disconnected!"); }, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
               {
                 Serial.print("Access point has started with ip: ");
                 Serial.println(WiFi.softAPIP()); },
               ARDUINO_EVENT_WIFI_AP_START);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
               { Serial.println("Access point has stopped!"); },
               ARDUINO_EVENT_WIFI_AP_STOP);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
               { Serial.println("Station connected to Access Point"); },
               ARDUINO_EVENT_WIFI_AP_STACONNECTED);

  WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info)
               { Serial.println("Station disconnected from Access Point"); },
               ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");

  if (isRunning)
  {

    backend->begin();
  }
}

void WiFiTask::setLedDuty(AsyncWebServerRequest *request)
{
  if (mode::HTML::HTMLLedControl)
  {

    if ((!request->hasParam("id")) || (!request->hasParam("value")))
    {
      request->send(406, "text/plain", "No arguments!");
      return;
    }

    uint8_t id = request->getParam("id")->value().toInt();
    float value = request->getParam("value")->value().toFloat();

    ((mode::HTML *)current_mode)->setDuty(id, value);

    request->send(200, "OK");
  }

  request->send(406, "HTML control disabled");
}

void WiFiTask::setLedFreq(AsyncWebServerRequest *request)
{
  if (mode::HTML::HTMLLedControl)
  {

    if ((!request->hasParam("id")) || (!request->hasParam("value")))
    {
      request->send(406, "text/plain", "No arguments!");
      return;
    }

    uint8_t id = request->getParam("id")->value().toInt();
    float value = request->getParam("value")->value().toFloat();

    ((mode::HTML *)current_mode)->setFrequency(id, value);

    request->send(200, "OK");
  }

  request->send(406, "HTML control disabled");
}

void WiFiTask::clearSSID(AsyncWebServerRequest *request)
{
  // clear all SSID from json
  if (LittleFS.remove("/wlans.json"))
  {
    request->send(200, "OK");
    Serial.println("/ssid_cls ssids has been removed!");
  }
  else
  {
    request->send(404, "text/plain", "No file!");
    Serial.println("/ssid_cls no file found!");
  }
}

void WiFiTask::removeSSID(AsyncWebServerRequest *request)
{
  // remove SSID from json

  if (!request->hasHeader("ssid"))
  {
    request->send(406, "text/plain", "No arguments!");
    Serial.println("/ssid_rem no arguments!");
    return;
  }

  const AsyncWebHeader *head = request->getHeader("ssid");

  char ssid[SSID_LINE_LENGTH];
  memset(ssid, 0, SSID_LINE_LENGTH);
  strcpy(ssid, head->value().c_str());

  Serial.print("/ssid_rem - remove network with ssid: ");
  Serial.println(ssid);

  if (!remove_ssid_json(ssid))
  {
    Serial.println("/ssid_rem - cannot remove network!");
    request->send(406, "text/plain", "Cannot remove network!");
    return;
  }

  Serial.println("/ssid_rem - network has been removed!");
  FindConnection = true;
  request->send(200, "OK");
}

void WiFiTask::registerSSID(AsyncWebServerRequest *request)
{
  // register SSID with password

  if ((!request->hasHeader("ssid")) || (!request->hasHeader("password")))
  {
    request->send(406, "text/plain", "No arguments!");
    Serial.println("/ssid_add no arguments!");
    return;
  }

  const AsyncWebHeader *ssid_head = request->getHeader("ssid");

  char _ssid[SSID_LINE_LENGTH];
  memset(_ssid, 0, SSID_LINE_LENGTH);
  strcpy(_ssid, ssid_head->value().c_str());

  const AsyncWebHeader *pwd_head = request->getHeader("password");

  char _pwd[SSID_LINE_LENGTH];
  memset(_pwd, 0, SSID_LINE_LENGTH);
  strcpy(_pwd, pwd_head->value().c_str());

  Serial.print("/ssid_add - adding network with ssid: ");
  Serial.println(_ssid);
  Serial.println(_pwd);

  if (!add_json_ssid(_ssid, _pwd))
  {
    Serial.println("/ssid_add - cannot save network data!");
    request->send(404, "text/plain", "Cannot save network data!");
    return;
  }

  Serial.println("/ssid_add - network has been added!");

  request->send(200, "text/plain", "Network has been added!");

  FindConnection = true;
  to_timeout = millis();
  memset(ssid, 0, SSID_LINE_LENGTH * sizeof(char));
  memset(pwd, 0, SSID_LINE_LENGTH * sizeof(char));
}

void WiFiTask::updateSSID(AsyncWebServerRequest *request)
{
  if ((!request->hasHeader("ssid")) || (!request->hasHeader("password")))
  {
    request->send(406, "text/plain", "No arguments!");
    Serial.println("/ssid_up no arguments!");
    return;
  }

  const AsyncWebHeader *ssid_head = request->getHeader("ssid");

  char _ssid[SSID_LINE_LENGTH];
  memset(_ssid, 0, SSID_LINE_LENGTH);
  strcpy(_ssid, ssid_head->value().c_str());

  const AsyncWebHeader *pass_head = request->getHeader("password");

  char _pwd[SSID_LINE_LENGTH];
  memset(_pwd, 0, SSID_LINE_LENGTH);
  strcpy(_pwd, pass_head->value().c_str());

  if (!check_file_for_ssid(_ssid))
  {
    Serial.print("/ssid_up no network with ssid: ");
    Serial.print(_ssid);
    request->send(404, "text/plain", "Cannot update network data!");
    return;
  }

  if (!add_json_ssid(_ssid, _pwd))
  {
    Serial.println("/ssid_up - cannot update network data!");
    request->send(404, "text/plain", "Cannot update network data!");
    return;
  }

  Serial.print("/ssid_up network password with ssid: ");
  Serial.print(_ssid);
  Serial.println(" updated!");
  FindConnection = true;
  request->send(200, "OK");
}

void WiFiTask::getSSID(AsyncWebServerRequest *request)
{

  if (!request->hasParam("index"))
  {
    request->send(406, "text/plain", "No arguments!");
    Serial.println("/ssid_get no arguments!");
    return;
  }

  const AsyncWebParameter *head = request->getParam("index");

  uint32_t index = std::stoi(head->value().c_str());

  Serial.println("/ssid_get - parameters:");
  Serial.print("index: ");
  Serial.println(index);

  StaticJsonDocument<WIFI_JSON_BUFFER> doc;

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  JsonObject obj = doc.createNestedObject();

  WiFiTask::PackSSID(obj, index);

  Serial.println("/ssid_get - sending the response");

  response->setCode(200);

  serializeJson(doc, *response);

  request->send(response);
}

void WiFiTask::beginScanSSID(AsyncWebServerRequest *request)
{

  if (!Connecting)
  {

    Serial.println("/ssid begining scanning!");

    startScanning = true;

    WiFi.scanDelete();
  }

  request->send(200, "OK");
}

void WiFiTask::scanSSID(AsyncWebServerRequest *request)
{
  // get n ssids from index

  if ((!request->hasParam("index")) || (!request->hasParam("counts")))
  {
    request->send(406, "text/plain", "No arguments!");
    Serial.println("/ssid no arguments!");
    return;
  }

  const AsyncWebParameter *index_param = request->getParam("index");

  uint32_t index = std::stoi(index_param->value().c_str());

  const AsyncWebParameter *counts_param = request->getParam("counts");

  uint32_t counts = std::stoi(counts_param->value().c_str());

  Serial.println("/ssid - parameters:");
  Serial.print("index: ");
  Serial.println(index);
  Serial.print("counts: ");
  Serial.println(counts);

  int16_t discoveredWlans = WiFi.scanComplete();

  if (discoveredWlans < 0)
  {
    request->send(406, "application/json", "{}");

    Serial.println("/ssid - no ssid found");
    return;
  }

  if (index >= discoveredWlans)
  {
    request->send(406, "application/json", "{}");

    Serial.println("/ssid - index out of range");
    return;
  }

  StaticJsonDocument<WIFI_JSON_BUFFER> doc;

  // JsonArray array = doc.to<JsonArray>();

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  if (WiFi.status() == WL_CONNECTED)
  {
    JsonObject obj = doc.createNestedObject();

    WiFiTask::PackSSID(obj);
  }

  for (uint32_t i = index; i < discoveredWlans; i++)
  {

    // WiFiTask::PackSSID(obj,i);
    if (WiFi.SSID() != WiFi.SSID(i))
    {

      JsonObject obj = doc.createNestedObject();

      WiFiTask::PackSSID(obj, i);
    }

    if (i >= (index + counts))
    {
      break;
    }
  }

  Serial.println("/ssid - sending the response");

  response->setCode(200);

  serializeJson(doc, *response);

  request->send(response);
}

void WiFiTask::listSSID(AsyncWebServerRequest *request)
{
  // get ssid from flash memory
  if ((!request->hasParam("index")) || (!request->hasParam("counts")))
  {
    request->send(406, "text/plain", "No arguments!");
    Serial.println("/ssid_list no arguments!");
    return;
  }

  const AsyncWebParameter *index_param = request->getParam("index");

  uint32_t index = std::stoi(index_param->value().c_str());

  const AsyncWebParameter *counts_param = request->getParam("counts");

  uint32_t counts = std::stoi(counts_param->value().c_str());

  Serial.println("/ssid_list - parameters:");
  Serial.print("index: ");
  Serial.println(index);
  Serial.print("counts: ");
  Serial.println(counts);

  StaticJsonDocument<WIFI_JSON_BUFFER> doc;

  // request->send(LittleFS,"/wlans.json");

  fs::File file = LittleFS.open("/wlans.json", FILE_READ);

  if (!file)
  {
    request->send(406, "application/json", "{}");

    Serial.println("/ssid_list - no file");
    return;
  }

  //  Serial.println(file);

  DeserializationError error = deserializeJson(doc, file);

  if (error)
  {
    request->send(406, "application/json", "{}");

    Serial.println("/ssid_list - json deserialization error");
    return;
  }

  file.close();

  if (index >= doc.size())
  {
    request->send(406, "application/json", "{}");

    Serial.println("/ssid_list - index out of range");
    return;
  }

  StaticJsonDocument<WIFI_JSON_BUFFER> output;

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  ;

  // JsonArray array = doc.to<JsonArray>();

  JsonObject json_keys = doc.as<JsonObject>();

  if (WiFi.status() == WL_CONNECTED)
  {

    JsonObject obj = output.createNestedObject();

    WiFiTask::PackSSID(obj);
  }

  uint32_t i = 0;

  for (auto ite : json_keys)
  {
    if ((i >= index) && (WiFi.SSID() != ite.key().c_str()))
    {
      JsonObject obj = output.createNestedObject();

      WiFiTask::PackSSID(obj, ite.key().c_str());

      if (i > (index + counts))
      {
        break;
      }
    }

    i++;
  }

  Serial.println("/ssid_list - sending the response");

  response->setCode(200);

  serializeJson(output, *response);

  request->send(response);
}

void WiFiTask::disconnectSSID(AsyncWebServerRequest *request)
{
  Serial.println("/ssid_disc - disconnecting...");
  // WiFi.disconnect();

  toDisconnect = true;

  request->send(200, "OK");
}

void WiFiTask::handleRoot(AsyncWebServerRequest *request)
{
  // main page
  Serial.println("/ - sending root page");

  if (mode::HTML::HTMLLedControl)
  {
    request->send(200, "text/html", ledPageHTML);
  }
  else
  {
    request->send(200, "text/html", index_html);
  }
}

void WiFiTask::handleCSS(AsyncWebServerRequest *request)
{
  Serial.println("/ - sending css file");
  request->send(200, "text/css", style_css);
}

// a javascript
void WiFiTask::handleJS(AsyncWebServerRequest *request)
{
  Serial.println("/ - sending js script file");
  request->send(200, "text/javascript", script_js);
}

void WiFiTask::handle404(AsyncWebServerRequest *request)
{
  // 404 error page

  if (request->method() == HTTP_OPTIONS)
  {
    request->send(200, "OK");
  }
  else
  {

    request->send(404, "text/plain", "Not found");
  }
}

void WiFiTask::ParameterNumber(AsyncWebServerRequest *request)
{
  AsyncResponseStream *response = request->beginResponseStream("text/plain");

  Serial.print("/param_n - parameter number ");
  response->setCode(200);

  Serial.println(params.size());
  response->print(params.size());

  request->send(response);
}

void WiFiTask::readParameter(AsyncWebServerRequest *request)
{
  if (!request->hasParam("id"))
  {

    Serial.println("/readP - no id parameter!");
    request->send(406, "application/json", "{}");
    return;
  }

  const AsyncWebParameter *_id = request->getParam("id");

  const uint32_t id = std::stoi(_id->value().c_str());

  if (params.empty())
  {
    Serial.println("readP - no parameters!");
    request->send(404, "application/json", "{}");
    return;
  }

  if (id >= params.size())
  {
    Serial.println("/readP - id out of range!");
    request->send(404, "application/json", "{}");
    return;
  }

  Serial.println("/readP - parameter is in preparation...");

  Parameter *out_param = params[id];

  StaticJsonDocument<WIFI_JSON_BUFFER> doc;

  AsyncResponseStream *response = request->beginResponseStream("application/json");

  response->setCode(200);

  out_param->generateResponse(response);

  request->send(response);

  Serial.println("/readP - parameter has been send!");
}

void WiFiTask::setParameter(AsyncWebServerRequest *request)
{
  if (!request->hasParam("id"))
  {

    Serial.println("/setP - no id parameter!");
    request->send(406, "text/plain", "no id parameter!");
    return;
  }

  if (!request->hasHeader("value"))
  {

    Serial.println("/setP - no value header!");
    request->send(406, "text/plain", "no value header!");
    return;
  }

  if (params.empty())
  {
    Serial.println("readP - no parameters!");
    request->send(404, "text/plain", "no parameters!");
    return;
  }

  const AsyncWebParameter *_id_param = request->getParam("id");

  const uint32_t id = std::stoi(_id_param->value().c_str());

  if (id >= params.size())
  {
    Serial.println("/setP - id out of range!");
    request->send(404, "text/plain", "id out of range!");
    return;
  }

  const AsyncWebHeader *_value_head = request->getHeader("value");

  const char *value = _value_head->value().c_str();

  Parameter *_param = params[id];

  _param->update(value);

  Serial.print("/setP - parameter with id: ");
  Serial.print(id);
  Serial.println("has been updated!");

  request->send(200, "text/plain", "Parameter has been updated!");
}

void WiFiTask::UpdateParameter(AsyncWebServerRequest *request)
{
  Serial.println("/upP - updating parameters...");

  SaveParameters = true;

  request->send(200, "text/plain", "OK");
}

void WiFiTask::addParameter(Parameter *_param)
{
  params.push_back(_param);
}

void WiFiTask::clearParameters()
{
  params.clear();
}

void WiFiTask::addSaveCallback(SaveCallback _callback)
{
  saveCallback = _callback;
}

void WiFiTask::StopWebServer()
{
  backend->end();
  isRunning = false;
}

void WiFiTask::StartWebServer()
{
  backend->begin();
  isRunning = true;
}

bool WiFiTask::WebState()
{
  return isRunning;
}