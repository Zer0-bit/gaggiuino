#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"

AsyncCallbackJsonWebHandler* jsonHandler(const char* uri, WebRequestMethodComposite method, ArJsonRequestHandlerFunction onRequest);

void onJsonBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

ArRequestHandlerFunction withJson(ArJsonRequestHandlerFunction wrappedHandler, size_t bufferSize = 1024);
#endif
