#include "api_wifi.h"

#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>
#include "../utils/server_utils.h"
#include "../../wifi/wifi_setup.h"
#include "../../log/log.h"

void handlePostSelectedNetwork(AsyncWebServerRequest* request, JsonVariant& body);
void handleGetNetworks(AsyncWebServerRequest* request);
void handleDeleteSelectedNetwork(AsyncWebServerRequest* request);
void handleGetWifiStatus(AsyncWebServerRequest* request);
void handleRefreshNetworks(AsyncWebServerRequest* request);

void setupWifiApi(AsyncWebServer& server) {
  server.on("/api/wifi/status", HTTP_GET, handleGetWifiStatus);
  server.on("/api/wifi/networks", HTTP_GET, handleGetNetworks);
  server.on("/api/wifi/selected-network", HTTP_DELETE, handleDeleteSelectedNetwork);
  server.on("/api/wifi/networks", HTTP_DELETE, handleRefreshNetworks);
  server.addHandler(jsonHandler("/api/wifi/selected-network", HTTP_PUT, handlePostSelectedNetwork));
}

void sendWifiConnectResponse(AsyncWebServerRequest* request, uint16_t code, String message);
void handlePostSelectedNetwork(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to connect to WiFi");

  String ssid = body["ssid"].as<String>();
  String pass = body["pass"].as<String>();
  if (ssid.isEmpty()) {
    sendWifiConnectResponse(request, 422, "SSID cannot be empty");
    return;
  }

  if (!wifiConnect(ssid, pass, 9000)) {
    sendWifiConnectResponse(request, 422, "Could not connect to " + ssid + ". Check your password");
    return;
  }

  sendWifiConnectResponse(request, 200, "Success!");
}

void handleGetNetworks(AsyncWebServerRequest* request) {
  LOG_INFO("Got request get available WiFi networks");
  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(2048);
  JsonArray networksJson = json.to<JsonArray>();

  for (auto& network : wifiAvailableNetworks()) {
    JsonObject jsonNetwork = networksJson.createNestedObject();
    jsonNetwork["ssid"] = network.ssid;
    jsonNetwork["rssi"] = network.rssi;
    jsonNetwork["secured"] = network.secured;
  }

  serializeJson(networksJson, *response);
  request->send(response);
}

void handleDeleteSelectedNetwork(AsyncWebServerRequest* request) {
  LOG_INFO("Got request to disconnect connect from WiFi");
  AsyncWebServerResponse* response = request->beginResponse(204);
  request->send(response);
  wifiDisconnect();
}

void handleGetWifiStatus(AsyncWebServerRequest* request) {
  LOG_INFO("Got request to get WiFi status");
  AsyncResponseStream* response = request->beginResponseStream("application/json");
  StaticJsonDocument<256> json;
  WiFiConnection connection = getWiFiConnection();
  json["ssid"] = connection.ssid;
  json["ip"] = connection.ip;
  json["status"] = connection.ip == "" ? "disconnected" : "connected";
  serializeJson(json, *response);
  request->send(response);
}

void handleRefreshNetworks(AsyncWebServerRequest* request) {
  LOG_INFO("Got request to refresh WiFi networks");
  wifiRefreshNetworks();
  sendWifiConnectResponse(request, 200, "Success!");
}

void sendWifiConnectResponse(AsyncWebServerRequest* request, uint16_t code, String message) {
  AsyncResponseStream* response = request->beginResponseStream("application/json");
  StaticJsonDocument<256> json;
  json["result"] = code < 300 ? "ok" : "error";
  json["message"] = message;
  response->setCode(code);
  serializeJson(json, *response);
  request->send(response);
}
