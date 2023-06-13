#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "ESPAsyncWebServer.h"
#include "AsyncJson.h"

AsyncCallbackJsonWebHandler* jsonHandler(const char* uri, WebRequestMethodComposite method, ArJsonRequestHandlerFunction onRequest) {
  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler(String(uri), onRequest);
  handler->setMethod(method);
  return handler;
}


#endif
