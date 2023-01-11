#ifndef WIFI_API_H
#define WIFI_API_H

#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>
#include "../wifi/wifi_setup.h"

void handlePostSelectedNetwork(AsyncWebServerRequest* request);
void handleGetNetworks(AsyncWebServerRequest* request);
void handleDeleteSelectedNetwork(AsyncWebServerRequest* request);
void handleGetWifiStatus(AsyncWebServerRequest* request);

void setupWifiApi(AsyncWebServer& server) {
  server.on("/api/wifi/status", HTTP_GET, handleGetWifiStatus);
  server.on("/api/wifi/networks", HTTP_GET, handleGetNetworks);
  server.on("/api/wifi/selected-network", HTTP_PUT, handlePostSelectedNetwork);
  server.on("/api/wifi/selected-network", HTTP_DELETE, handleDeleteSelectedNetwork);
}

void handlePostSelectedNetwork(AsyncWebServerRequest* request) {
  Serial.println("Got request to connect to WiFi");
  int params = request->params();
  String ssid;
  String pass;

  for (int i = 0;i < params;i++) {
    AsyncWebParameter* p = request->getParam(i);
    if (p->isPost()) {
      // HTTP POST ssid value
      if (p->name() == PARAM_INPUT_SSID) {
        ssid = p->value();
        Serial.printf("SSID set to: %s\n", ssid);
      }
      // HTTP POST pass value
      if (p->name() == PARAM_INPUT_PASS) {
        pass = p->value();
        Serial.printf("Password set to: %s\n", "*************");
      }
    }
  }

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(256);
  if (wifiConnect(ssid, pass, 9000)) {
    json["result"] = "ok";
    json["message"] = "Done. Connected to WiFi.";
    response->setCode(200);
  } else {
    json["result"] = "failed";
    json["message"] = "Failed to connect to WiFi.";
    response->setCode(422);
  }
  serializeJson(json, *response);
  request->send(response);
}

void handleGetNetworks(AsyncWebServerRequest* request) {
  Serial.println("Got request get available WiFi networks");
  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(2048);
  JsonArray networksJson = json.to<JsonArray>();

  for (int i = 0; i < wifiNetworkCount(); i++) {
    JsonObject network = networksJson.createNestedObject();
    network["ssid"] = WiFi.SSID(i);
    network["rssi"] = WiFi.RSSI(i);
    network["secured"] = WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? false : true;
  }

  serializeJson(networksJson, *response);
  request->send(response);
}

void handleDeleteSelectedNetwork(AsyncWebServerRequest* request) {
  Serial.println("Got request to disconnect connect from WiFi");
  AsyncWebServerResponse* response = request->beginResponse(204);
  request->send(response);
  wifiDisconnect();
}

void handleGetWifiStatus(AsyncWebServerRequest* request) {
  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(256);
  if (WiFi.isConnected()) {
    json["status"] = "connected";
    json["ssid"] = WiFi.SSID();
    json["ip"] = WiFi.localIP().toString();
  } else {
    json["status"] = "disconnected";
    json["ssid"] = "";
    json["ip"] = "";
  }
  serializeJson(json, *response);
  request->send(response);
}


#endif

