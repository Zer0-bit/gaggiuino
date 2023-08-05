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

void handleGetScalesSettings(AsyncWebServerRequest* request);
void handlePutScalesSettings(AsyncWebServerRequest* request, JsonVariant& body);

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

  server.on("/api/settings/scales", HTTP_GET, handleGetScalesSettings);
  server.on("/api/settings/scales", HTTP_PUT, withJson(handlePutScalesSettings), NULL, onJsonBody);

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

  DynamicJsonDocument json(100);
  JsonObject jsonObj = json.to<JsonObject>();
  json::mapLedSettingsToJson(led, jsonObj);

  sendJsonResponse(request, jsonObj);
}

void handleGetBoilerSettings(AsyncWebServerRequest* request) {
  const BoilerSettings& boiler = state::getSettings().boiler;

  LOG_INFO("Got request get BoilerSettings");

  DynamicJsonDocument json(100);
  JsonObject jsonObj = json.to<JsonObject>();
  json::mapBoilerSettingsToJson(boiler, jsonObj);

  sendJsonResponse(request, jsonObj);
}


void handleGetBrewSettings(AsyncWebServerRequest* request) {
  const BrewSettings& brew = state::getSettings().brew;

  LOG_INFO("Got request get BrewSettings");

  DynamicJsonDocument json(100);
  JsonObject jsonObj = json.to<JsonObject>();
  json::mapBrewSettingsToJson(brew, jsonObj);

  sendJsonResponse(request, jsonObj);
}

void handleGetSystemSettings(AsyncWebServerRequest* request) {
  const SystemSettings& system = state::getSettings().system;

  LOG_INFO("Got request get SystemSettings");

  DynamicJsonDocument json(100);
  JsonObject jsonObj = json.to<JsonObject>();
  json::mapSystemSettingsToJson(system, jsonObj);

  sendJsonResponse(request, jsonObj);
}

void handleGetScalesSettings(AsyncWebServerRequest* request) {
  const ScalesSettings& scales = state::getSettings().scales;

  LOG_INFO("Got request get ScalesSettings");

  DynamicJsonDocument json(400);
  JsonObject jsonObj = json.to<JsonObject>();
  json::mapScalesSettingsToJson(scales, jsonObj);

  sendJsonResponse(request, jsonObj);
}

void handleGetAllSettings(AsyncWebServerRequest* request) {
  const GaggiaSettings& settings = state::getSettings();

  LOG_INFO("Got request get all settings");

  DynamicJsonDocument json(500);
  JsonObject jsonObj = json.to<JsonObject>();
  json::mapAllSettingsToJson(settings, jsonObj);

  sendJsonResponse(request, jsonObj);
}

// ------------------------------------------------------------------------------------------
// ------------------------------------- PUT METHODS ----------------------------------------
// ------------------------------------------------------------------------------------------

void handlePutAllSettings(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update all settings");

  state::updateAllSettings(json::mapJsonToAllSettings(body));

  JsonObject responseBody = body.to<JsonObject>();
  json::mapAllSettingsToJson(state::getSettings(), responseBody);

  sendJsonResponse(request, responseBody);
}

void handlePutBoilerSettings(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update BoilerSettings");

  state::updateBoilerSettings(json::mapJsonToBoilerSettings(body));

  JsonObject responseBody = body.to<JsonObject>();
  json::mapBoilerSettingsToJson(state::getSettings().boiler, responseBody);

  sendJsonResponse(request, responseBody);
}

void handlePutBrewSettings(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update BrewSettings");

  state::updateBrewSettings(json::mapJsonToBrewSettings(body));

  JsonObject responseBody = body.to<JsonObject>();
  json::mapBrewSettingsToJson(state::getSettings().brew, responseBody);

  sendJsonResponse(request, responseBody);
}

void handlePutLedSettings(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update LedSettings");

  state::updateLedSettings(json::mapJsonToLedSettings(body));

  JsonObject responseBody = body.to<JsonObject>();
  json::mapLedSettingsToJson(state::getSettings().led, responseBody);

  sendJsonResponse(request, responseBody);
}

void handlePutSystemSettings(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update SystemSettings");

  state::updateSystemSettings(json::mapJsonToSystemSettings(body));

  JsonObject responseBody = body.to<JsonObject>();
  json::mapSystemSettingsToJson(state::getSettings().system, responseBody);

  sendJsonResponse(request, responseBody);
}

void handlePutScalesSettings(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update ScalesSettings");

  state::updateScalesSettings(json::mapJsonToScalesSettings(body));

  JsonObject responseBody = body.to<JsonObject>();
  json::mapScalesSettingsToJson(state::getSettings().scales, responseBody);

  sendJsonResponse(request, responseBody);
}


void handlePersistSettings(AsyncWebServerRequest* request) {
  LOG_INFO("Got request to persist current settings");

  AsyncResponseStream* response = request->beginResponseStream("application/json");

  state::persistSettings();

  response->setCode(200);
  request->send(response);
}
