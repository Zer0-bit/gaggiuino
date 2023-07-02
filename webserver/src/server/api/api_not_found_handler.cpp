#include "api_not_found_handler.h"
#include "LittleFS.h"
#include "../../log/log.h"

bool shouldServeIndexHtml(String path);
void serveNotFound(AsyncWebServerRequest* request);

void handleUrlNotFound(AsyncWebServerRequest* request) {
  if (!request->method() == HTTP_GET) {
    serveNotFound(request);
    return;
  }

  // Handle redirects to index.hml to support React Single Page App
  if (shouldServeIndexHtml(request->url())) {
    LOG_VERBOSE("Not found %s. Redirecting to /index.html", request->url().c_str());
    request->send(LittleFS, "/index.html", "text/html");
    return;
  }

  LOG_VERBOSE("Not found %s. Serving 404 response.", request->url().c_str());
  serveNotFound(request);
}

bool shouldServeIndexHtml(String path) {
  // If the path begin swith /api we don't redirect
  if (path.startsWith("/api")) {
    LOG_DEBUG("%s starts with /api", path.c_str());
    return false;
  }
  // If the path has a file format ending with a dot suffix we don't redirect
  // example: /path/to/file.jpg
  int dotIndex = path.lastIndexOf('.');
  int slashIndex = path.lastIndexOf('/');
  if (dotIndex > 0 && dotIndex > slashIndex && dotIndex < path.length() - 1) {
    LOG_DEBUG("%s contains file suffix. dotIndex=%d, slashIndex=%d, pathLen=%d", path.c_str(), dotIndex, slashIndex, path.length());
    return false;
  }

  // Otherwise we serve index.html to support single page React app
  // Example: User goes to /settings or /profiles.
  // We want to server index.html and let the React app handle the routing
  return true;
}

void serveNotFound(AsyncWebServerRequest* request) {
    AsyncResponseStream* response = request->beginResponseStream("application/text");
    response->setCode(404);
    response->printf("%s not found", request->url().c_str());
}
