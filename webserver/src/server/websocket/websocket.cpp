#include "websocket.h"
#include <deque>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>
#include "../../log/log.h"

#include "../json/json_system_state_converters.h"

const std::string WS_MSG_SENSOR_DATA = "sensor_data_update";
const std::string WS_MSG_SHOT_DATA = "shot_data_update";
const std::string WS_MSG_LOG = "log_record";
const std::string WS_MSG_SYSTEM_STATE = "log_sys_state";

namespace websocket {
  AsyncWebSocket wsServer("/ws");
  SemaphoreHandle_t jsonMutex = xSemaphoreCreateRecursiveMutex();
  DynamicJsonDocument jsonDoc(2048);

  bool lockJson() {
    return xSemaphoreTakeRecursive(jsonMutex, portMAX_DELAY) == pdTRUE;
  }

  void unlockJson() {
    jsonDoc.clear();
    xSemaphoreGiveRecursive(jsonMutex);
  }

  std::deque<std::string> msgBuffer;
  SemaphoreHandle_t bufferLock = xSemaphoreCreateRecursiveMutex();

  void wsSendWithBuffer(std::string message) {
    if (xSemaphoreTakeRecursive(bufferLock, portMAX_DELAY) == pdFALSE) return;

    msgBuffer.push_back(message);
    while (msgBuffer.size() > 50) {
      msgBuffer.pop_front();
    }

    while (wsServer.count() > 0 && !msgBuffer.empty()) {
      wsServer.textAll(msgBuffer.front().c_str(), msgBuffer.front().length());
      msgBuffer.pop_front();
    }
    xSemaphoreGiveRecursive(bufferLock);
  }
}

void handleWebSocketMessage(void* arg, uint8_t* data, size_t len);
void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

void setupWebSocket(AsyncWebServer& server) {
  websocket::wsServer.onEvent(&onEvent);
  server.addHandler(&websocket::wsServer);
}

void wsCleanup() {
  websocket::wsServer.cleanupClients();
}

void onEvent(
  AsyncWebSocket* server,
  AsyncWebSocketClient* client,
  AwsEventType          type,
  void* arg,
  uint8_t* data,
  size_t                len
) {
  switch (type) {
  case WS_EVT_CONNECT:
    LOG_INFO("WebSocket client #%u connected from %s", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    LOG_INFO("WebSocket client #%u disconnected", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

//-------------------------------------------------------------//
//--------------------------INCOMING---------------------------//
//-------------------------------------------------------------//

void handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
  AwsFrameInfo* info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    if (!websocket::lockJson()) return;

    DeserializationError err = deserializeJson(websocket::jsonDoc, data);
    if (err) {
      LOG_ERROR("deserializeJson() failed with code %s", err.c_str());
      websocket::unlockJson();
      return;
    }

    const std::string action = websocket::jsonDoc["action"].as<std::string>();
    const std::string actionData = websocket::jsonDoc["data"].as<std::string>();
    LOG_INFO("Message: %s -> %s\n", action.c_str(), actionData.c_str());
    websocket::unlockJson();
  }
}

//-------------------------------------------------------------//
//--------------------------OUTGOING---------------------------//
//-------------------------------------------------------------//
void wsSendSensorStateSnapshotToClients(const SensorStateSnapshot& snapshot) {
  if (!websocket::lockJson()) return;
  JsonObject root = websocket::jsonDoc.to<JsonObject>();

  root["action"] = WS_MSG_SENSOR_DATA;

  JsonObject data = root.createNestedObject("data");
  data["brewActive"] = snapshot.brewActive;
  data["steamActive"] = snapshot.steamActive;
  data["hotWaterActive"] = snapshot.hotWaterSwitchState;
  data["temperature"] = snapshot.temperature;
  data["waterTemperature"] = snapshot.waterTemperature;
  data["pressure"] = snapshot.pressure;
  data["pumpFlow"] = snapshot.pumpFlow;
  data["weightFlow"] = snapshot.weightFlow;
  data["weight"] = snapshot.weight;
  data["waterLevel"] = snapshot.waterLevel;

  std::string serializedMsg; // create temp buffer
  serializeJson(root, serializedMsg);  // serialize to buffer
  websocket::unlockJson();

  websocket::wsServer.textAll(serializedMsg.c_str(), serializedMsg.length());
}

void wsSendShotSnapshotToClients(const ShotSnapshot& snapshot) {
  if (!websocket::lockJson()) return;
  JsonObject root = websocket::jsonDoc.to<JsonObject>();

  root["action"] = WS_MSG_SHOT_DATA;

  JsonObject data = root.createNestedObject("data");
  data["timeInShot"] = snapshot.timeInShot;
  data["pressure"] = snapshot.pressure;
  data["pumpFlow"] = snapshot.pumpFlow;
  data["weightFlow"] = snapshot.weightFlow;
  data["temperature"] = snapshot.temperature;
  data["shotWeight"] = snapshot.shotWeight;
  data["waterPumped"] = snapshot.waterPumped;
  data["targetTemperature"] = snapshot.targetTemperature;
  data["targetPumpFlow"] = snapshot.targetPumpFlow;
  data["targetPressure"] = snapshot.targetPressure;

  std::string serializedMsg; // create temp buffer
  serializeJson(root, serializedMsg);  // serialize to buffer
  websocket::unlockJson();

  websocket::wsServer.textAll(serializedMsg.c_str(), serializedMsg.length());
}

void wsSendLog(std::string log, std::string source) {
  if (!websocket::lockJson()) return;
  JsonObject root = websocket::jsonDoc.to<JsonObject>();

  root["action"] = WS_MSG_LOG;

  JsonObject data = root.createNestedObject("data");
  data["source"] = source;
  data["log"] = log;

  std::string serializedMsg; // create temp buffer
  serializeJson(root, serializedMsg);  // serialize to buffer
  websocket::unlockJson();

  websocket::wsSendWithBuffer(serializedMsg);
}

void wsSendSystemStateToClients(const SystemState& systemState) {
  if (!websocket::lockJson()) return;
  JsonObject root = websocket::jsonDoc.to<JsonObject>();

  root["action"] = WS_MSG_SYSTEM_STATE;

  JsonObject data = root.createNestedObject("data");
  json::mapSystemStateToJson(systemState, data);

  std::string serializedMsg; // create temp buffer
  serializeJson(root, serializedMsg);  // serialize to buffer
  websocket::unlockJson();

  websocket::wsServer.textAll(serializedMsg.c_str(), serializedMsg.length());
}
