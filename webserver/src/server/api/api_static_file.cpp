#include "api_static_files.h"
#include <LittleFS.h>
#include "../../log/log.h"

class GzipFileResponse: public AsyncFileResponse {
  using headers_t = LinkedList<AsyncWebHeader *>;

  public:
    GzipFileResponse(fs::FS &fs, const String& path, const String& contentType)
    : AsyncFileResponse(fs, path, contentType, false, AwsTemplateProcessor()) {

      _headers.remove_first([](const AsyncWebHeader* header) {
        return header->name() == "Content-Disposition";
      });

      String originalPath = path;
      originalPath.replace(".gz", "");
      String disp = "inline; filename=\"" + originalPath.substring(originalPath.lastIndexOf('/') + 1) + "\"";
      addHeader("Content-Disposition", disp);
      addHeader("Content-Encoding", "gzip");
    }
};

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
