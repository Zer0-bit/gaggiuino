#include "api_settings.h"
#include "../utils/server_utils.h"
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>

#include "../../state/state.h"
#include "../../persistence/persistence.h"
#include "../../log/log.h"
#include "../json/json_profile_converters.h"

// Gets and updates part of the settings object in memory.

void handleGetProfileSummaries(AsyncWebServerRequest* request);
void handleGetProfileById(AsyncWebServerRequest* request);
void handleGetActiveProfile(AsyncWebServerRequest* request);

void handlePostProfile(AsyncWebServerRequest* request, JsonVariant& body);
void handleUpdateProfile(AsyncWebServerRequest* request, JsonVariant& body);
void handleDeleteProfile(AsyncWebServerRequest* request);

void handleSelectActiveProfileId(AsyncWebServerRequest* request, JsonVariant& body);
void handleGetActiveProfileId(AsyncWebServerRequest* request);
void handlePersistActiveProfile(AsyncWebServerRequest* request);
void handleUpdateActiveProfile(AsyncWebServerRequest* request, JsonVariant& body);

void setupProfileApi(AsyncWebServer& server) {
  server.on("/api/profile-summaries", HTTP_GET, handleGetProfileSummaries);
  server.on("^\\/api\\/profiles\\/([0-9]+)$", HTTP_GET, handleGetProfileById);
  server.on("^\\/api\\/profiles\\/([0-9]+)$", HTTP_DELETE, handleDeleteProfile);
  server.on("^\\/api\\/profiles\\/([0-9]+)$", HTTP_PUT, withJson(handleUpdateProfile, 4096), NULL, onJsonBody);
  server.on("/api/profiles", HTTP_POST, withJson(handlePostProfile, 4096), NULL, onJsonBody);

  server.on("/api/profiles/active-profile/id", HTTP_PUT, withJson(handleSelectActiveProfileId, 50), NULL, onJsonBody);
  server.on("/api/profiles/active-profile/id", HTTP_GET, handleGetActiveProfileId);
  server.on("/api/profiles/active-profile/persist", HTTP_PUT, handlePersistActiveProfile);
  server.on("/api/profiles/active-profile", HTTP_GET, handleGetActiveProfile);
  server.on("/api/profiles/active-profile", HTTP_PUT, withJson(handleUpdateActiveProfile, 4096), NULL, onJsonBody);
}

// ------------------------------------------------------------------------------------------
// ------------------------------- Profile Summaries ----------------------------------------
// ------------------------------------------------------------------------------------------

void handleGetProfileSummaries(AsyncWebServerRequest* request) {
  LOG_INFO("Got request to get profile summaries");

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(2048);
  JsonArray profileSummariesJson = json.to<JsonArray>();

  auto savedProfiles = persistence::getSavedProfiles();
  for (auto savedProfile : savedProfiles) {
    auto savedProfileJson = profileSummariesJson.createNestedObject();
    json::mapProfileSummaryToJson(savedProfile, savedProfileJson);
  }

  serializeJson(profileSummariesJson, *response);
  request->send(response);
}

// ------------------------------------------------------------------------------------------
// ----------------------------- Profile actions by ID --------------------------------------
// ------------------------------------------------------------------------------------------
void handleGetProfileById(AsyncWebServerRequest* request) {
  LOG_INFO("Got request to get profile by id");

  uint32_t id = request->pathArg(0).toInt();
  auto result = persistence::getProfile(id);
  if (!result.first) {
    request->send(404);
    return;
  }

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(4096);
  JsonObject jsonObj = json.to<JsonObject>();

  json::mapProfileToJson(id, result.second, jsonObj);
  serializeJson(jsonObj, *response);
  request->send(response);
}

void handlePostProfile(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to create new profile");

  Profile newProfile = json::mapJsonToProfile(body);
  auto result = persistence::saveNewProfile(newProfile);
  if (!result.first) {
    request->send(422);
    return;
  }
  SavedProfile savedProfile = result.second;

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  body["id"] = savedProfile.id;

  serializeJson(body, *response);
  request->send(response);
}

void handleUpdateProfile(AsyncWebServerRequest* request, JsonVariant& body) {
  uint32_t id = request->pathArg(0).toInt();
  LOG_INFO("Got request to update profile by id %d", id);

  if (!persistence::profileExists(id)) {
    request->send(404);
    return;
  }

  Profile profile = json::mapJsonToProfile(body);
  if (id == state::getActiveProfileId()) {
    state::updateActiveProfile(profile);
  }
  persistence::saveProfile(id, profile);

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  body["id"] = id;

  serializeJson(body, *response);
  request->send(response);
}

void handleDeleteProfile(AsyncWebServerRequest* request) {
  LOG_INFO("Got request to delete profile by id");

  uint32_t id = request->pathArg(0).toInt();

  if (state::getActiveProfileId() == id || persistence::getActiveProfileId() == id) {
    request->send(422);
    return;
  }

  bool result = persistence::deleteProfile(id);
  if (!result) {
    request->send(422);
    return;
  }

  request->send(200);
}

// ------------------------------------------------------------------------------------------
// ----------------------------- Active profile actions -------------------------------------
// ------------------------------------------------------------------------------------------
void handleGetActiveProfile(AsyncWebServerRequest* request) {
  LOG_INFO("Got request to get active profile");

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(4096);
  JsonObject jsonObj = json.to<JsonObject>();

  json::mapProfileToJson(state::getActiveProfileId(), state::getActiveProfile(), jsonObj);
  serializeJson(jsonObj, *response);
  request->send(response);
}

void handleSelectActiveProfileId(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to select new active profile");

  uint32_t id = body["id"];

  bool result = state::updateActiveProfileId(id) && state::persistActiveProfileId();
  if (!result) {
    request->send(422, "application/json", "{}");
    return;
  }

  request->send(200);
}

void handleGetActiveProfileId(AsyncWebServerRequest* request) {
  LOG_INFO("Got request to get the active profile id");

  AsyncResponseStream* response = request->beginResponseStream("application/json");
  DynamicJsonDocument json(50);
  JsonObject jsonObj = json.to<JsonObject>();
  jsonObj["id"] = state::getActiveProfileId();

  serializeJson(jsonObj, *response);
  request->send(response);
}

void handlePersistActiveProfile(AsyncWebServerRequest* request) {
  LOG_INFO("Got request to persist active profile");

  bool result = state::persistActiveProfile();

  if (result) {
    request->send(200);
  }
  else {
    request->send(422);
  }
}

void handleUpdateActiveProfile(AsyncWebServerRequest* request, JsonVariant& body) {
  LOG_INFO("Got request to update active profile");
  AsyncResponseStream* response = request->beginResponseStream("application/json");

  state::updateActiveProfile(json::mapJsonToProfile(body));
  body["id"] = state::getActiveProfileId();

  serializeJson(body, *response);
  request->send(response);
}
