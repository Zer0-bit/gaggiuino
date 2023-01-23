#include "wifi_setup.h"
#include "esp_task_wdt.h"

const char* PARAM_INPUT_SSID = "ssid";
const char* PARAM_INPUT_PASS = "pass";
WiFiParams_t wifiParams;

void setupWiFiAccessPoint();

WiFiParams_t& getWifiParams() {
  return wifiParams;
}

void wifiInit();

void wifiSetup() {
  WiFi.mode(WIFI_AP_STA);
  wifiParams.preferences.begin("gaggiuino_wifi");
  wifiInit();
  setupWiFiAccessPoint();
}

// Initialize WiFi
void wifiInit() {
  if (wifiParams.ssid == "" && wifiParams.pass == "") {
    wifiParams.ssid = wifiParams.preferences.getString(PARAM_INPUT_SSID);
    wifiParams.pass = wifiParams.preferences.getString(PARAM_INPUT_PASS);
  }

  Serial.printf("initWifi: status=[%d], ssid=[%s], ip=[%s].\n", WiFi.status(), WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

  if (wifiParams.ssid == "" && wifiParams.pass == "") {
    Serial.println("No ssid or password provided.");
    return;
  }

  wifiConnect(wifiParams.ssid, wifiParams.pass);
}

bool wifiConnect(String ssid, String pass, const unsigned long timeout) {
  unsigned long wifiStartTimer = millis();

  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.printf("Connecting to WiFi [%s]\n", ssid.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("\nFailed to connect. Check password.");
      return false;
    }
    if (millis() - wifiStartTimer >= timeout) {
      Serial.printf("\nFailed to connect after %ld seconds.\n", timeout / 1000);
      return false;
    }
    esp_task_wdt_reset();
    delay(200);
  }

  Serial.printf("\nConnected to WiFi [%s] with IP:[%s]\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

  wifiParams.ssid = ssid;
  wifiParams.pass = pass;
  wifiParams.preferences.putString(PARAM_INPUT_SSID, ssid);
  wifiParams.preferences.putString(PARAM_INPUT_PASS, pass);

  wifiParams.attemptReconnect = false;
  return true;
}

void wifiScanNetworks() {
  wifiParams.wifiNetworkCount = WiFi.scanNetworks();
  if (wifiParams.wifiNetworkCount != WIFI_SCAN_FAILED) {
    wifiParams.refreshWiFiNetworks = false;
  }
}

int wifiNetworkCount() {
  return wifiParams.wifiNetworkCount;
}

void setupWiFiAccessPoint() {
  // Connect to Wi-Fi network with SSID and password
  WiFi.softAP("Gaggiuino AP", NULL);
  Serial.printf("AP (Access Point) IP address: %s\n", WiFi.softAPIP().toString().c_str());
  wifiScanNetworks();
}

void wifiDisconnect() {
  if (WiFi.isConnected()) {
    WiFi.disconnect();
    wifiParams.ssid = "";
    wifiParams.pass = "";
    wifiParams.attemptReconnect = true;
    wifiParams.preferences.clear();
    Serial.println("Disconnected from WiFi and cleared saved WiFi.");
  }
}
