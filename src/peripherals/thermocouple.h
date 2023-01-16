#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include "pindef.h"

#if defined TEMP_SENSOR_TSIC_30x
  #include <TSIC.h>
  TSIC tempsensor(tempsensorPin, NO_VCC_PIN, TSIC_30x);    // Signalpin, VCCpin, Sensor Type
#elif defined SINGLE_BOARD
  #include <Adafruit_MAX31855.h>
  Adafruit_MAX31855 thermocouple(thermoCLK, thermoCS, thermoDO);
#else
  #include <max6675.h>
  MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
#endif

static inline void tempsensorInit(void) {
#if defined SINGLE_BOARD
  thermocouple.begin();
#endif
}

static inline float readCelsius(void) {
#if defined TEMP_SENSOR_TSIC_30x
  uint16_t temperature = 0;
  if (tempsensor.getTemperature(&temperature)) {
    return tempsensor.calc_Celsius(&temperature);
  }
  return NAN;
#else
  return thermocouple.readCelsius();
#endif
}

#endif
