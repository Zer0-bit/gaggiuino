#include "api_settings.h"
#include "../utils/server_utils.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>

#include "../json/json_settings_converters.h"
#include "../../state/state.h"
#include "../../log/log.h"

// Gets and updates part of the settings object in memory.
void handleGetConnectedScales(AsyncWebServerRequest* request);
void handleGetAvailableScales(AsyncWebServerRequest* request);

void setupScalesApi(AsyncWebServer& server) {
  server.on("/api/bt-scales/connected", HTTP_GET, handleGetConnectedScales);
  server.on("/api/bt-scales/available", HTTP_GET, handleGetAvailableScales);
}

// ------------------------------------------------------------------------------------------
// ------------------------------------- GET METHODS ----------------------------------------
// ------------------------------------------------------------------------------------------

void handleGetConnectedScales(AsyncWebServerRequest* request) {
  auto connectedScales = blescales::getConnectedScales();

  LOG_INFO("Got request get connected scales");

  DynamicJsonDocument json(100);
  JsonObject jsonObj = json.to<JsonObject>();

  json::mapBleScalesToJson(connectedScales, jsonObj);

  sendJsonResponse(request, jsonObj);
}

void handleGetAvailableScales(AsyncWebServerRequest* request) {
  auto availableScales = blescales::getAvailableScales();

  LOG_INFO("Got request get available scales");

  DynamicJsonDocument json(2048);
  JsonArray jsonArray = json.to<JsonArray>();

  for (auto scales : availableScales) {
    JsonObject scalesJson = jsonArray.createNestedObject();
    json::mapBleScalesToJson(scales, scalesJson);
  }

  sendJsonResponse(request, jsonArray);
}
