#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H
#include <Arduino.h>
#include <list>
#include <Preferences.h>

/** Holds information about one of the available networks */
struct WiFiNetwork {
  String ssid;
  int32_t rssi;
  bool secured;
};

/** Holds the current network and IP */
struct WiFiConnection {
  String ssid;
  String ip;
};

// WiFi definitions
void wifiSetup();
bool wifiConnect(String ssid, String pass, const uint32_t timeout = 15000);
void wifiDisconnect();
void wifiRefreshNetworks();

WiFiConnection getWiFiConnection();
std::list<WiFiNetwork> wifiAvailableNetworks();

#endif
