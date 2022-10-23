#ifndef SERVER_H
#define SERVER_H
#include <Arduino.h>
#include "secrets.h"
#include "static.h"

long getSeconds(void);
long getMinutes(void);

#define FORMAT_LITTLEFS_IF_FAILED   true

#endif
