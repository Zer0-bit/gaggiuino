#include "api_system_state.h"
#include "../utils/server_utils.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>

#include "../json/json_system_state_converters.h"
#include "../../state/state.h"
#include "../../log/log.h"

// Gets and updates part of the settings object in memory.
void handleGetSystemState(AsyncWebServerRequest* request);
void handleUpdateSystemStateOperationMode(AsyncWebServerRequest* request, JsonVariant& json);
void handleUpdateSystemStateTarePending(AsyncWebServerRequest* request, JsonVariant& json);

void setupSystemStateApi(AsyncWebServer& server) {
  server.on("/api/system-state", HTTP_GET, handleGetSystemState);
  server.on("/api/system-state/operation-mode", HTTP_PUT, withJson(handleUpdateSystemStateOperationMode), NULL, onJsonBody);
  server.on("/api/system-state/tare-pending", HTTP_PUT, withJson(handleUpdateSystemStateTarePending), NULL, onJsonBody);
}

// ------------------------------------------------------------------------------------------
// ------------------------------------- GET METHODS ----------------------------------------
// ------------------------------------------------------------------------------------------

void systemStateResponse(AsyncWebServerRequest* request, JsonObject json, int code = 200) {
  json::mapSystemStateToJson(state::getSystemState(), json);
  sendJsonResponse(request, json, code);
}

void handleGetSystemState(AsyncWebServerRequest* request) {
  LOG_INFO("Got request get operation mode");

  DynamicJsonDocument json(512);
  JsonObject jsonObj = json.to<JsonObject>();
  systemStateResponse(request, jsonObj);
}

void handleUpdateSystemStateTarePending(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update operation mode");

  state::updateTarePending(body["tarePending"]);

  JsonObject responseBody = body.to<JsonObject>();
  systemStateResponse(request, responseBody);
}

void handleUpdateSystemStateOperationMode(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update operation mode");

  state::updateOperationMode(json::mapJsonValueToOperationMode(body["operationMode"]));

  JsonObject responseBody = body.to<JsonObject>();
  systemStateResponse(request, responseBody);
}
