#ifndef THERMOCOUPLE_H
#define THERMOCOUPLE_H

#include "pindef.h"

#ifdef SINGLE_BOARD
  #include <Adafruit_MAX31855.h>
  Adafruit_MAX31855 thermocouple(thermoCLK, thermoCS, thermoDO);
#else
  #include <max6675.h>
  MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
#endif

static inline void thermocoupleInit(void) {
#if defined(MAX31855_ENABLED)
  thermocouple.begin();
#endif
}

#endif
