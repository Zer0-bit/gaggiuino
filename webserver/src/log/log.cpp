/* 09:32 15/03/2023 - change triggering comment */
#include "./log.h"
#include <Arduino.h>

namespace logging {
  void (*remoteLogCallback)(std::string message);
  HardwareSerial debugPort(0);
}

void log_init() {
  logging::debugPort.begin(115200);
}

void remote_log_init(void (*remoteLogCallback)(std::string message)) {
  logging::remoteLogCallback = remoteLogCallback;
}

void trim_newline(std::string &s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {
        s.pop_back();
    }
}

// Unsure the consequences of swapping to parameter packing or currying since
// this would then need to be moved to the header. So suppressing this for now.
void log(const char* prefix, const char* file, const int line, const char* msg, ...) //NOLINT(cert-dcl50-cpp)
{
  char msgBuf[LOG_MAX_STRING_LEN];
  va_list args;
  va_start(args, msg);
  size_t check = vsnprintf(msgBuf, LOG_MAX_STRING_LEN, msg, args);
  va_end(args);

  std::string formatted_msg(msgBuf);
  trim_newline(formatted_msg);  // remove trailing newline

  if (check > 0 && check <= LOG_MAX_STRING_LEN) {
    char logLineBuf[LOG_MAX_PREFIX_LEN + LOG_MAX_STRING_LEN];
    check = snprintf(logLineBuf, sizeof(logLineBuf), "%s (%s:%i): %s", prefix, file, line, formatted_msg.c_str());
    if (check > 0 && check <= sizeof(logLineBuf)) {
      logging::debugPort.println(logLineBuf);
    }
    if (logging::remoteLogCallback != nullptr) {
      logging::remoteLogCallback(std::string(logLineBuf));
    }
  }
}
