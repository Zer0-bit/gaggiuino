#include "wifi_setup.h"
#include <WiFi.h>
#include "../log/log.h"

WiFiParams wifiParams;

void setupWiFiAccessPoint();
void wifiScanNetworks();
void wifiInit();

void wifiSetup() {
  while(!WiFi.mode(WIFI_AP_STA)) {
    LOG_INFO("Initialising WiFi mode.");
    delay(500);
  }
  LOG_INFO("Initialised WiFi in AP+STA mode.");
  setupWiFiAccessPoint();
  wifiInit();
  wifiScanNetworks();
}

// Initialize WiFi
void wifiInit() {
  wifiParams.init();

  if (!wifiParams.hasCredentials()) {
    LOG_INFO("No ssid or password provided.");
    return;
  }

  LOG_INFO("Will attempt connecting to %s - %s", wifiParams.getSSID().c_str(), "***");
  wifiConnect(wifiParams.getSSID(), wifiParams.getPass());
}

bool wifiConnect(String ssid, String pass, const uint32_t timeout) {
  if (WiFi.isConnected()) {
    WiFi.disconnect();
  }

  WiFi.begin(ssid.c_str(), pass.c_str());
  LOG_INFO("Connecting to WiFi [%s]", ssid.c_str());

  if (WiFi.waitForConnectResult(timeout) != WL_CONNECTED) {
    LOG_INFO("Failed to connect. Check password.");
    return false;
  }

  LOG_INFO("Connected to WiFi [%s] with IP:[%s]", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  wifiParams.saveCredentials(ssid, pass);

  return true;
}

namespace ws_wifi {
  uint32_t lastWiFiScan = millis() + 5000;
  bool newScanResults = false;
  std::vector<WiFiNetwork> wifiNetworks;
  const uint32_t WIFI_SCAN_EVERY_MS = 60000;
}

void wifiScanNetworks() {
  // If scan is still running, do nothing
  if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) {
    ws_wifi::newScanResults = true;
    return;
  }

  if (WiFi.scanComplete() >= 0 && ws_wifi::newScanResults) {
    LOG_INFO("WiFi scan completed. Updating networks in RAM");
    ws_wifi::wifiNetworks.clear();
    for (int i = 0; i < WiFi.scanComplete(); i++) {
      ws_wifi::wifiNetworks.push_back(WiFiNetwork{
        .ssid = WiFi.SSID(i),
        .rssi = WiFi.RSSI(i),
        .secured = WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? false : true,
        });
      ws_wifi::newScanResults = false;
    }
  }

  // If scan failed or completed too long ago started again
  if (millis() - ws_wifi::lastWiFiScan > ws_wifi::WIFI_SCAN_EVERY_MS) {
    LOG_INFO("Starting a new WiFi scan");

    WiFi.scanNetworks(true);
    ws_wifi::lastWiFiScan = millis();
    ws_wifi::newScanResults = true;
  }
}

std::vector<WiFiNetwork> wifiAvailableNetworks() {
  return ws_wifi::wifiNetworks;
}

void setupWiFiAccessPoint() {
  WiFi.softAP("Gaggiuino AP", NULL);
  LOG_INFO("AP (Access Point) IP address: %s", WiFi.softAPIP().toString().c_str());
  delay(50);
}

WiFiConnection getWiFiConnection() {
  if (WiFi.isConnected()) {
    return WiFiConnection{.ssid= WiFi.SSID(), .ip=WiFi.localIP().toString() };
  } else {
    return WiFiConnection{.ssid= "", .ip="" };
  }
}

void wifiDisconnect() {
  if (WiFi.isConnected()) {
    WiFi.disconnect();
    wifiParams.reset();
    LOG_INFO("Disconnected from WiFi and cleared saved WiFi.");
  }
}

uint32_t wifiConnectionMaintenanceTimer = 0;
void wifiMaintainConnection() {
  if (WiFi.isConnected() || !wifiParams.hasCredentials() || millis() - wifiConnectionMaintenanceTimer < 10000) {
    return;
  }
  wifiConnectionMaintenanceTimer = millis();

  LOG_INFO("WiFi disconnected, will attempt reconnection to %s", wifiParams.getSSID().c_str());
  WiFi.begin(wifiParams.getSSID().c_str(), wifiParams.getPass().c_str());
}

void wifiUpdate() {
  wifiScanNetworks();
  wifiMaintainConnection();
}

// ----------------------------------------------------
// ------------------ WiFiParams ----------------------
// ----------------------------------------------------

void WiFiParams::saveCredentials(String ssid, String pass) {
  this->ssid = ssid;
  this->pass = pass;
  preferences.putString("ssid", ssid.c_str());
  preferences.putString("pass", pass.c_str());
  LOG_INFO("Saved wifi credentials [%s, %s]", ssid.c_str(), "*****");
}

void WiFiParams::init() {
  preferences.begin("gaggiuino_wifi");
  if (!hasCredentials()) {
    this->ssid = preferences.getString("ssid", "");
    this->pass = preferences.getString("pass", "");
  }
}

void WiFiParams::reset() {
  ssid = "";
  pass = "";
  preferences.clear();
}
