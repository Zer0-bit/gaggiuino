#include "log.h"

void log_init() {
  USART_DEBUG.begin(115200);
}

void log(const char *prefix, const char *file, const int line, const char *msg, ...)
{
  char msgBuf[LOG_MAX_STRING_LEN];
  va_list args;
  va_start(args, msg);
  vsnprintf(msgBuf, LOG_MAX_STRING_LEN, msg, args);
  va_end(args);

  char logLineBuf[LOG_MAX_PREFIX_LEN + LOG_MAX_STRING_LEN];
  snprintf(logLineBuf, sizeof(logLineBuf), "%s (%s:%i): %s", prefix, file, line, msgBuf);

  USART_DEBUG.println(logLineBuf);
}
