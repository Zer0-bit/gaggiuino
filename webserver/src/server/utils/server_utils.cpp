#include "server_utils.h"

#define WS_MAX_JSON_CONTENT_LENGTH 16384

AsyncCallbackJsonWebHandler* jsonHandler(const char* uri, WebRequestMethodComposite method, ArJsonRequestHandlerFunction onRequest) {
  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler(String(uri), onRequest);
  handler->setMethod(method);
  return handler;
}

void onJsonBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  if (total > 0 && request->_tempObject == NULL && total < WS_MAX_JSON_CONTENT_LENGTH) {
    request->_tempObject = malloc(total);
  }
  if (request->_tempObject != NULL) {
    memcpy((uint8_t*)(request->_tempObject) + index, data, len);
  }
}

ArRequestHandlerFunction withJson(ArJsonRequestHandlerFunction wrappedHandler, size_t bufferSize) {
  return [wrappedHandler, bufferSize](AsyncWebServerRequest *request) {
    DynamicJsonDocument jsonBuffer(bufferSize);
    DeserializationError error = deserializeJson(jsonBuffer, (uint8_t*)(request->_tempObject));
    if(!error) {
      JsonVariant json = jsonBuffer.as<JsonVariant>();
      wrappedHandler(request, json);
      return;
    }
  };
}
