#include "websocket.h"
#include "../../log/log.h"
#include <deque>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "AsyncTCP.h"
#include <ArduinoJson.h>

const std::string WS_MSG_SENSOR_DATA = "sensor_data_update";
const std::string WS_MSG_SHOT_DATA = "shot_data_update";
const std::string WS_MSG_LOG = "log_record";

namespace websocket {
  AsyncWebSocket wsServer("/ws");

  std::deque<std::string> msgBuffer;
  void wsSendWithBuffer(std::string message) {
    msgBuffer.push_back(message);
    while (msgBuffer.size() > 50) {
      msgBuffer.pop_front();
    }

    while (wsServer.count() > 0 && !msgBuffer.empty()) {
      wsServer.textAll(msgBuffer.front().c_str(), msgBuffer.front().length());
      msgBuffer.pop_front();
    }
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
    StaticJsonDocument<1024> json;

    DeserializationError err = deserializeJson(json, data);
    if (err) {
      LOG_ERROR("deserializeJson() failed with code %s", err.c_str());
      return;
    }

    const std::string action = json["action"].as<std::string>();
    const std::string actionData = json["data"].as<std::string>();
    LOG_INFO("Message: %s -> %s\n", action.c_str(), actionData.c_str());
  }
}

//-------------------------------------------------------------//
//--------------------------OUTGOING---------------------------//
//-------------------------------------------------------------//
void wsSendSensorStateSnapshotToClients(SensorStateSnapshot& snapshot) {
  StaticJsonDocument<512> json;
  JsonObject root = json.to<JsonObject>();

  root["action"] = WS_MSG_SENSOR_DATA;

  JsonObject data = root.createNestedObject("data");
  data["brewActive"] = snapshot.brewActive;
  data["steamActive"] = snapshot.steamActive;
  data["scalesPresent"] = snapshot.scalesPresent;
  data["temperature"] = snapshot.temperature;
  data["pressure"] = snapshot.pressure;
  data["pumpFlow"] = snapshot.pumpFlow;
  data["weightFlow"] = snapshot.weightFlow;
  data["weight"] = snapshot.weight;

  std::string serializedMsg; // create temp buffer
  serializeJson(root, serializedMsg);  // serialize to buffer
  websocket::wsServer.textAll(serializedMsg.c_str(), serializedMsg.length());
}

void wsSendShotSnapshotToClients(ShotSnapshot& snapshot) {
  StaticJsonDocument<512> json;
  JsonObject root = json.to<JsonObject>();

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
  websocket::wsServer.textAll(serializedMsg.c_str(), serializedMsg.length());
}

void wsSendLog(std::string log, std::string source) {
  DynamicJsonDocument json(256);
  JsonObject root = json.to<JsonObject>();

  root["action"] = WS_MSG_LOG;

  JsonObject data = root.createNestedObject("data");
  data["source"] = source;
  data["log"] = log;

  std::string serializedMsg; // create temp buffer
  serializeJson(root, serializedMsg);  // serialize to buffer
  websocket::wsSendWithBuffer(serializedMsg);
}
