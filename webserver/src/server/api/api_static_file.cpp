#include "api_static_files.h"
#include <LittleFS.h>
#include "../../log/log.h"

void setupStaticFiles(AsyncWebServer& server) {

  // Prefer .gz files for .js
  // server.on("^\\/assets\\/.*\\.(js|html)$", HTTP_GET, [](AsyncWebServerRequest *request){
  //   String path = request->url();
  //   String gzipPath = path + ".gz";

  //   String encoding = request->header("Accept-Encoding");
  //   if (encoding.indexOf("gzip") != -1 && LittleFS.exists(gzipPath)) {

  //     String contentType = "application/javascript";
  //     if (path.endsWith(".html")) contentType = "text/html";

  //     AsyncWebServerResponse *response = request->beginResponse(LittleFS, gzipPath, contentType);
  //     response->addHeader("Content-Encoding", "gzip");
  //     request->send(response);
  //   } else {
  //     request->send(LittleFS, path);
  //   }
  // });

  server.serveStatic("/", LittleFS, "/");
    // .setDefaultFile("index.html")
    // .setCacheControl("max-age=600");
}
