#ifndef MEASUREMENTS_UTIL_H
#define MEASUREMENTS_UTIL_H

#include "Arduino.h"
#include <deque>

/** Holds a measurement value and the time it was taken */
struct Measurement {
  float value;
  uint32_t millis;
};

/** Holds a measurement delta and can calculate its speed of change */
struct MeasurementChange {
  float deltaValue;
  uint32_t deltaMillis;

  float changeSpeed() {
    if (deltaMillis == 0) {
      return 0.f;
    }
    return 1000.f * deltaValue / static_cast<float>(deltaMillis);
  }
};

/**
  * Holds a defined amount of measurements (size).
  * This class can be used to calculate the speed of change of a measured value
  * when the values change slower than how frequently we check.
  *
  * Example: We are taking the weight measurement (speed of weight measurement) every 100msec
  * but remote scales are sending a new weight every 200msec or more. So 2 consecutive weight checks
  * may end up having the same value. To prevent this we can store th last N values and
  * calculate the speed of change of the measurement by finding the closest different value.
  */
class Measurements {
private:
  size_t size;
  std::deque<Measurement> values;
public:
  Measurements(size_t size) : size(size) {}

  Measurement latest();
  Measurement previous();
  MeasurementChange measurementChange();
  void clear();
  void add(float value);
  void add(Measurement measurement);
};

#endif
