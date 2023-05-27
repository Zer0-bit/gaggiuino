#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H
#include <Arduino.h>
#include <vector>
#include <Preferences.h>

struct WiFiNetwork {
  String ssid;
  int32_t rssi;
  bool secured;
};

struct WiFiConnection {
  String ssid;
  String ip;
};

class WiFiParams {
private:
  String ssid = "";
  String pass = "";
  Preferences preferences;
public:
  String getSSID() { return ssid; }
  String getPass() { return pass;}
  bool hasCredentials() { return ssid != ""; };
  void saveCredentials(String ssid, String pass);
  void init();
  void reset();
};

// WiFi definitions
void wifiSetup();
bool wifiConnect(String ssid, String pass, const uint32_t timeout = 15000);
WiFiConnection getWiFiConnection();
void wifiDisconnect();
void wifiUpdate();

std::vector<WiFiNetwork> wifiAvailableNetworks();

#endif
