#include "wifi_setup.h"
#include <WiFi.h>
#include "../task_config.h"
#include "../log/log.h"

/**
 * Helper class for persisting and retrieving WiFi connection credentials
 */
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

namespace wifi {
  WiFiParams credentials;
  SemaphoreHandle_t lock = xSemaphoreCreateRecursiveMutex();
  std::vector<WiFiNetwork> networks;

  uint32_t lastScan;
  const uint32_t SCAN_EVERY_MS = 10000;

  uint32_t lastMaintainConnection;
  const uint32_t MAINTAIN_CONNECTION_EVERY = 10000;

  bool newScanResults = false;
}

void setupWiFiAccessPoint();
void wifiTask(void* params);
void wifiInit();

void wifiSetup() {
  WiFi.mode(WIFI_AP_STA);
  setupWiFiAccessPoint();
  wifiInit();

  xTaskCreateUniversal(&wifiTask, "wifi_maintenance", configMINIMAL_STACK_SIZE + 2048, NULL, PRIORITY_WIFI_MAINTENANCE, NULL, CORE_WIFI_MAINTENANCE);
}

std::list<WiFiNetwork> wifiAvailableNetworks() {
  std::list<WiFiNetwork> copy;
  if (xSemaphoreTakeRecursive(wifi::lock, portMAX_DELAY) == pdPASS) {
    for (auto& network : wifi::networks) {
      copy.push_back(network);
    }
    xSemaphoreGiveRecursive(wifi::lock);
  }
  return copy;
}

WiFiConnection getWiFiConnection() {
  if (WiFi.isConnected()) {
    return WiFiConnection{ .ssid = WiFi.SSID(), .ip = WiFi.localIP().toString() };
  }
  else {
    return WiFiConnection{ .ssid = "", .ip = "" };
  }
}

void wifiDisconnect() {
  if (xSemaphoreTakeRecursive(wifi::lock, portMAX_DELAY) == pdFALSE) return;

  if (WiFi.isConnected()) {
    WiFi.disconnect();
    wifi::credentials.reset();
    LOG_INFO("Disconnected from WiFi and cleared saved WiFi.");
  }

  xSemaphoreGiveRecursive(wifi::lock);
}

void setupWiFiAccessPoint() {
  WiFi.softAP("Gaggiuino AP", NULL);
  LOG_INFO("AP (Access Point) IP address: %s", WiFi.softAPIP().toString().c_str());
}

void wifiMaintainConnection();

void wifiTask(void* params) {
  wifi::lastScan = 0;
  wifiRefreshNetworks();

  wifi::lastMaintainConnection = millis();
  while (true) {
    wifiMaintainConnection();
    delay(500);
  }
}

// Initialize WiFi. If we have persisted credentials then attempt connection
void wifiInit() {
  wifi::credentials.init();

  if (!wifi::credentials.hasCredentials()) {
    LOG_INFO("No ssid or password provided.");
    return;
  }

  LOG_INFO("Will attempt connecting to %s - %s", wifi::credentials.getSSID().c_str(), "***");
  wifiConnect(wifi::credentials.getSSID(), wifi::credentials.getPass());
}

// Connect to WiFi with given credentials and save credential
// to persisted memory if successful
bool wifiConnect(String ssid, String pass, const uint32_t timeout) {
  if (xSemaphoreTakeRecursive(wifi::lock, portMAX_DELAY) == pdFALSE) return false;

  if (WiFi.isConnected()) {
    WiFi.disconnect();
  }

  WiFi.begin(ssid.c_str(), pass.c_str());
  LOG_INFO("Connecting to WiFi [%s]", ssid.c_str());

  if (WiFi.waitForConnectResult(timeout) != WL_CONNECTED) {
    LOG_INFO("Failed to connect. Check password.");
    xSemaphoreGiveRecursive(wifi::lock);
    return false;
  }

  LOG_INFO("Connected to WiFi [%s] with IP:[%s]", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
  wifi::credentials.saveCredentials(ssid, pass);

  xSemaphoreGiveRecursive(wifi::lock);
  return true;
}

// Periodically scan for networks in the surrounding area and persist them in
// a vector which we then use to serve available networks
void wifiRefreshNetworks() {
  LOG_INFO("Starting a new WiFi scan");

  uint16_t count = WiFi.scanNetworks(false, false, false, 100U);
  if (count >= 0 && xSemaphoreTakeRecursive(wifi::lock, portMAX_DELAY) == pdTRUE) {
    LOG_INFO("WiFi scan completed. Updating networks in RAM");
    wifi::networks.clear();
    for (int i = 0; i < WiFi.scanComplete(); i++) {
      wifi::networks.push_back(WiFiNetwork{ WiFi.SSID(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? false : true });
    }
    xSemaphoreGiveRecursive(wifi::lock);
  }
  wifi::lastScan = millis();
}

// Check periodically if we're still connected.
// If not attempt to reconnect
void wifiMaintainConnection() {
  if (WiFi.isConnected() || !wifi::credentials.hasCredentials() || millis() - wifi::lastMaintainConnection < 10000) {
    return;
  }
  LOG_INFO("WiFi disconnected, will attempt reconnection to %s", wifi::credentials.getSSID().c_str());

  wifi::lastMaintainConnection = millis();
  wifiConnect(wifi::credentials.getSSID(), wifi::credentials.getPass(), 9000);
}

// ----------------------------------------------------
// ------------------ WiFiParams ----------------------
// ----------------------------------------------------

void WiFiParams::saveCredentials(String ssid, String pass) {
  if (this->ssid == ssid && this->pass == pass) return;

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
