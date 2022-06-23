#include "log.h"

void log_init() {
  USART_DEBUG.begin(115200);
}

void log(const char *prefix, const char *file, const int line, const char *msg, ...)
{
  char buf[LOG_MAX_STRING_LEN];
  va_list args;
  va_start(args, msg);
  vsnprintf(buf, LOG_MAX_STRING_LEN, msg, args);
  va_end(args);

  String tmp = prefix;
  tmp += " (";
  tmp += file;
  tmp += ":";
  tmp += line;
  tmp += "): ";
  tmp += buf;

  USART_DEBUG.println(tmp);
}
