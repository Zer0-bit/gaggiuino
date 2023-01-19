#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H
#include <WiFi.h>
#include <Preferences.h>
#include <Arduino.h>

// WiFi definitions
struct WiFiParams_t {
  int wifiNetworkCount = 0;
  bool refreshWiFiNetworks = false;
  bool attemptReconnect = false;

  String ssid;
  String pass;
  Preferences preferences;
};

void wifiSetup();
bool wifiConnect(String ssid, String pass, const unsigned long timeout = 10000);
void wifiDisconnect();
void wiFiApSetup();
int  wifiNetworkCount();

extern const char* PARAM_INPUT_SSID;
extern const char* PARAM_INPUT_PASS;
#endif
