#include "api_system_state.h"
#include "../utils/server_utils.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>

#include "../json/json_system_state_converters.h"
#include "../../state/state.h"
#include "../../log/log.h"

// Gets and updates part of the settings object in memory.
void handleGetOperationMode(AsyncWebServerRequest* request);
void handleUpdateOperationMode(AsyncWebServerRequest* request, JsonVariant& json);

void setupSystemStateApi(AsyncWebServer& server) {
  server.on("/api/system-state/operation-mode", HTTP_GET, handleGetOperationMode);
  server.on("/api/system-state/operation-mode", HTTP_PUT, withJson(handleUpdateOperationMode), NULL, onJsonBody);
}

// ------------------------------------------------------------------------------------------
// ------------------------------------- GET METHODS ----------------------------------------
// ------------------------------------------------------------------------------------------

void handleGetOperationMode(AsyncWebServerRequest* request) {
  LOG_INFO("Got request get operation mode");

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(100);
  JsonObject jsonObj = json.to<JsonObject>();
  jsonObj["operationMode"] = json::mapOperationModeToJsonValue(state::getOperationMode());

  response->setCode(200);
  serializeJson(jsonObj, *response);
  request->send(response);
}

void handleUpdateOperationMode(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update operation mode");

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  UpdateOperationMode command = json::mapJsonToUpdateOperationMode(body);
  state::updateOperationMode(command.operationMode);

  response->setCode(200);
  serializeJson(body, *response);
  request->send(response);
}
