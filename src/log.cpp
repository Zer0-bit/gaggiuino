/* 09:32 15/03/2023 - change triggering comment */
#include "log.h"

void log_init() {
  USART_DEBUG.begin(115200);
}

// Unsure the consequences of swapping to parameter packing or currying since
// this would then need to be moved to the header. So suppressing this for now.
void log(const char* prefix, const char* file, const int line, const char* msg, ...) //NOLINT(cert-dcl50-cpp)
{
  char msgBuf[LOG_MAX_STRING_LEN];
  va_list args;
  va_start(args, msg);
  int check = vsnprintf(msgBuf, LOG_MAX_STRING_LEN, msg, args);
  if (check > 0 && static_cast<unsigned int>(check) <= sizeof(msgBuf))
    va_end(args);

  char logLineBuf[LOG_MAX_PREFIX_LEN + LOG_MAX_STRING_LEN];
  check = snprintf(logLineBuf, sizeof(logLineBuf), "%s (%s:%i): %s", prefix, file, line, msgBuf);
  if (check > 0 && static_cast<unsigned int>(check) <= sizeof(logLineBuf))
    USART_DEBUG.println(logLineBuf);
}
