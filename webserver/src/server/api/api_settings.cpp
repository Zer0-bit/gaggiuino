#include "api_settings.h"
#include "../utils/server_utils.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>

#include "../json/json_settings_converters.h"
#include "../../state/state.h"
#include "../../log/log.h"

// Gets and updates part of the settings object in memory.
void handleGetAllSettings(AsyncWebServerRequest* request);
void handlePutAllSettings(AsyncWebServerRequest* request, JsonVariant& body);

void handleGetBoilerSettings(AsyncWebServerRequest* request);
void handlePutBoilerSettings(AsyncWebServerRequest* request, JsonVariant& body);

void handleGetBrewSettings(AsyncWebServerRequest* request);
void handlePutBrewSettings(AsyncWebServerRequest* request, JsonVariant& body);

void handleGetLedSettings(AsyncWebServerRequest* request);
void handlePutLedSettings(AsyncWebServerRequest* request, JsonVariant& body);

void handleGetSystemSettings(AsyncWebServerRequest* request);
void handlePutSystemSettings(AsyncWebServerRequest* request, JsonVariant& body);

// Only this method persists to non volatile storage
void handlePersistSettings(AsyncWebServerRequest* request);

void setupSettingsApi(AsyncWebServer& server) {

  server.on("/api/settings/boiler", HTTP_GET, handleGetBoilerSettings);
  server.on("/api/settings/boiler", HTTP_PUT, withJson(handlePutBoilerSettings), NULL, onJsonBody);

  server.on("/api/settings/brew", HTTP_GET, handleGetBrewSettings);
  server.on("/api/settings/brew", HTTP_PUT, withJson(handlePutBrewSettings), NULL, onJsonBody);

  server.on("/api/settings/led", HTTP_GET, handleGetLedSettings);
  server.on("/api/settings/led", HTTP_PUT, withJson(handlePutLedSettings), NULL, onJsonBody);

  server.on("/api/settings/system", HTTP_GET, handleGetSystemSettings);
  server.on("/api/settings/system", HTTP_PUT, withJson(handlePutSystemSettings), NULL, onJsonBody);

  server.on("/api/settings/persist",  HTTP_PUT, handlePersistSettings);

  server.on("/api/settings", HTTP_GET, handleGetAllSettings);
  server.on("/api/settings", HTTP_PUT, withJson(handlePutAllSettings), NULL, onJsonBody);
}

// ------------------------------------------------------------------------------------------
// ------------------------------------- GET METHODS ----------------------------------------
// ------------------------------------------------------------------------------------------

void handleGetLedSettings(AsyncWebServerRequest* request) {
  const LedSettings& led = state::getSettings().led;

  LOG_INFO("Got request get LedSettings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(100);
  JsonObject jsonObj = json.to<JsonObject>();
  json::mapLedSettingsToJson(led, jsonObj);

  serializeJson(jsonObj, *response);
  request->send(response);
}

void handleGetBoilerSettings(AsyncWebServerRequest* request) {
  const BoilerSettings& boiler = state::getSettings().boiler;

  LOG_INFO("Got request get BoilerSettings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(100);
  JsonObject jsonObj = json.to<JsonObject>();
  json::mapBoilerSettingsToJson(boiler, jsonObj);

  serializeJson(jsonObj, *response);
  request->send(response);
}


void handleGetBrewSettings(AsyncWebServerRequest* request) {
  const BrewSettings& brew = state::getSettings().brew;

  LOG_INFO("Got request get BrewSettings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(100);
  JsonObject jsonObj = json.to<JsonObject>();
  json::mapBrewSettingsToJson(brew, jsonObj);

  serializeJson(jsonObj, *response);
  request->send(response);
}

void handleGetSystemSettings(AsyncWebServerRequest* request) {
  const SystemSettings& system = state::getSettings().system;

  LOG_INFO("Got request get SystemSettings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(100);
  JsonObject jsonObj = json.to<JsonObject>();
  json::mapSystemSettingsToJson(system, jsonObj);

  serializeJson(jsonObj, *response);
  request->send(response);
}

void handleGetAllSettings(AsyncWebServerRequest* request) {
  const GaggiaSettings& settings = state::getSettings();

  LOG_INFO("Got request get all settings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(500);
  JsonObject jsonObj = json.to<JsonObject>();
  json::mapAllSettingsToJson(settings, jsonObj);

  serializeJson(jsonObj, *response);
  request->send(response);
}

// ------------------------------------------------------------------------------------------
// ------------------------------------- PUT METHODS ----------------------------------------
// ------------------------------------------------------------------------------------------

void handlePutAllSettings(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update all settings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");

  state::updateAllSettings(json::mapJsonToAllSettings(body));

  response->setCode(200);
  serializeJson(body, *response);
  request->send(response);
}

void handlePutBoilerSettings(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update BoilerSettings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");

  state::updateBoilerSettings(json::mapJsonToBoilerSettings(body));

  response->setCode(200);
  serializeJson(body, *response);
  request->send(response);
}

void handlePutBrewSettings(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update BrewSettings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");

  state::updateBrewSettings(json::mapJsonToBrewSettings(body));

  response->setCode(200);
  serializeJson(body, *response);
  request->send(response);
}

void handlePutLedSettings(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update LedSettings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");

  state::updateLedSettings(json::mapJsonToLedSettings(body));

  response->setCode(200);
  serializeJson(body, *response);
  request->send(response);

}

void handlePutSystemSettings(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update SystemSettings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");

  state::updateSystemSettings(json::mapJsonToSystemSettings(body));

  response->setCode(200);
  serializeJson(body, *response);
  request->send(response);
}

void handlePersistSettings(AsyncWebServerRequest* request) {
  LOG_INFO("Got request to persist current settings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");

  state::persistSettings();

  response->setCode(200);
  request->send(response);
}
