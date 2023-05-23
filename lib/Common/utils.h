/* 09:32 15/03/2023 - change triggering comment */
#ifndef GAGGIA_UTILS_H
#define GAGGIA_UTILS_H

#include "Arduino.h"
#include <deque>
#include <type_traits>

enum class TransitionCurve {
  EASE_IN_OUT,
  EASE_IN,
  EASE_OUT,
  LINEAR,
  INSTANT,
};

float mapRange(float sourceNumber, float fromA, float fromB, float toA, float toB, int decimalPrecision, TransitionCurve transition = TransitionCurve::LINEAR);

struct Measurement {
  float value;
  uint32_t millis;
};

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
