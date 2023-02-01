#ifndef LOG_H
#define LOG_H

#include <Arduino.h>
#include "pindef.h"
#include <stdio.h>

#define FILENAME (__builtin_strrchr("/" __FILE__, '/') + 1)
#define LOG_MAX_STRING_LEN 64
#define LOG_MAX_PREFIX_LEN 64

#ifdef LOG_LEVEL
  void log(const char *prefix, const char *file, const int line, const char *msg, ...);
  void log_init(void);

  #define LOG_INIT() do { log_init(); } while (0)
#else
  #define LOG_INIT()
#endif

#if LOG_LEVEL > 0
  #define LOG_ERROR(msg, ...)    do { log("E", FILENAME, __LINE__, msg, ##__VA_ARGS__); } while (0)
#else
  #define LOG_ERROR(msg, ...)
#endif

#if LOG_LEVEL > 1
  #define LOG_INFO(msg, ...)     do { log("I", FILENAME, __LINE__, msg, ##__VA_ARGS__); } while (0)
#else
  #define LOG_INFO(msg, ...)
#endif

#if LOG_LEVEL > 2
  #define LOG_VERBOSE(msg, ...)  do { log("V", FILENAME, __LINE__, msg, ##__VA_ARGS__); } while (0)
#else
  #define LOG_VERBOSE(msg, ...)
#endif

#if LOG_LEVEL > 3
  #define LOG_DEBUG(msg, ...)    do { log("D", FILENAME, __LINE__, msg, ##__VA_ARGS__); } while (0)
#else
  #define LOG_DEBUG(msg, ...)
#endif

#endif
