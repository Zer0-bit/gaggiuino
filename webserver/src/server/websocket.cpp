#include "websocket.h"

AsyncWebSocket ws("/ws");

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(
  AsyncWebSocket       *server,
  AsyncWebSocketClient *client,
  AwsEventType          type,
  void                 *arg,
  uint8_t              *data,
  size_t                len
);

void setupWebSocket(AsyncWebServer& server) {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void wsCleanup() {
  ws.cleanupClients();
}

void onEvent(
  AsyncWebSocket       *server,
  AsyncWebSocketClient *client,
  AwsEventType          type,
  void                 *arg,
  uint8_t              *data,
  size_t                len
) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

    const uint8_t size = JSON_OBJECT_SIZE(1);
    StaticJsonDocument<size> json;
    DeserializationError err = deserializeJson(json, data);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return;
    }

    const char *action = json["action"];
    const char *actionData = json["data"];
    Serial.printf("Message: %s -> %s\n", action, actionData);
  }
}

//-------------------------------------------------------------//
//--------------------------OUTGOING---------------------------//
//-------------------------------------------------------------//
void wsSendSensorStatesToClients(
  long timeInShot,
  float temp,
  float pressure,
  float flow,
  float weight
) {
    DynamicJsonDocument doc(200); // fixed size
    JsonObject          root = doc.to<JsonObject>();

    root["action"] = "sensor_data_update";

    JsonObject data = root.createNestedObject("data");
    data["timeInShot"] = timeInShot;
    data["temp"] = temp;
    data["pressure"] = pressure;
    data["flow"] = flow;
    data["weight"] = weight;

    char   buffer[200]; // create temp buffer
    size_t len = serializeJson(root, buffer);  // serialize to buffer
    ws.textAll(buffer, len); // send buffer to web socket
}
