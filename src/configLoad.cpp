#include "config.hpp"

#include <LittleFS.h>
#include <FS.h>

#include "configLoad.hpp"

namespace config
{

  bool loadServerURL(char *url)
  {
    fs::File file = LittleFS.open(SERVER_CONFIG_FILENAME, "r");

    if (!file)
    {
      return false;
    }

    StaticJsonDocument<4096> json;

    DeserializationError error = deserializeJson(json, file);

    if (!(json.containsKey("url")))
    {
      return false;
    }

    strncpy(url, json["url"], 1024);

    file.close();

    return true;
  }

  bool loadNetworkConfig(char *ip, char *mask, char *gateway, char *dns)
  {
    fs::File file = LittleFS.open(NETWORK_CONFIG_FILENAME, "r");

    if (!file)
    {
      return false;
    }

    StaticJsonDocument<4096> json;

    DeserializationError error = deserializeJson(json, file);

    if (!((json.containsKey("ip")) && (json.containsKey("gateway")) && (json.containsKey("dns"))))
    {
      return false;
    }

    strncpy(ip, json["ip"], 16);
    strncpy(mask, json["mask"], 16);
    strncpy(gateway, json["gateway"], 16);
    strncpy(dns, json["dns"], 16);

    file.close();

    return true;
  }

  bool saveServerURL(const char *url)
  {

    fs::File file = LittleFS.open(SERVER_CONFIG_FILENAME, "w", true);

    if (!file)
    {
      return false;
    }

    StaticJsonDocument<4096> json;

    json["url"] = url;

    if (serializeJson(json, file) == 0)
    {
      return false;
    }

    file.close();

    return true;
  }

  bool saveNetworkConfig(const char *ip, const char *mask, const char *gateway, const char *dns)
  {

    fs::File file = LittleFS.open(NETWORK_CONFIG_FILENAME, "w", true);

    if (!file)
    {
      return false;
    }

    StaticJsonDocument<4096> json;

    json["ip"] = ip;
    json["mask"] = mask;
    json["gateway"] = gateway;
    json["dns"] = dns;

    if (serializeJson(json, file) == 0)
    {
      return false;
    }

    file.close();

    return true;
  }

  uint8_t *loadSSLFile()
  {
    fs::File file = LittleFS.open(SSL_CERT_FILENAME, "r");

    if (!file)
    {
      return NULL;
    }

    uint8_t *out = new uint8_t[file.size()]{0};

    file.read(out, file.size());

    file.close();

    return out;
  }

}