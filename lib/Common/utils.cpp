/* 09:32 15/03/2023 - change triggering comment */
#include "utils.h"
#include "math.h"

#ifndef M_PI
#define M_PI 3.14159265359
#endif

float percentageWithTransition(float pct, TransitionCurve transition);

float mapRange(float refNumber, float refStart, float refEnd, float targetStart, float targetEnd, int decimalPrecision, TransitionCurve transition) {
  float deltaRef = refEnd - refStart;
  float deltaTarget = targetEnd - targetStart;

  if (deltaRef == 0) {
    return targetEnd;
  }

  float pct = fmax(0.0f, fmin(1.0f, abs((refNumber - refStart) / deltaRef)));

  float finalNumber = targetStart + deltaTarget * percentageWithTransition(pct, transition);

  int calcScale = (int)pow(10, decimalPrecision >= 0 ? decimalPrecision : 1);
  return (float)round(finalNumber * calcScale) / calcScale;
}

float easeIn(float pct) {
  return powf(pct, 1.675f);
}

float easeOut(float pct) {
  return 1.f - powf(1.f - pct, 1.675f);
}

float easeInOut(float pct) {
  return 0.5f * (sinf((pct - 0.5f) * (float)M_PI) + 1.f);
}

float percentageWithTransition(float pct, TransitionCurve transition) {
  if (transition == TransitionCurve::LINEAR) {
    return pct;
  }
  else if (transition == TransitionCurve::EASE_IN) {
    return easeIn(pct);
  }
  else if (transition == TransitionCurve::EASE_OUT) {
    return easeOut(pct);
  }
  else if (transition == TransitionCurve::INSTANT) {
    return 1.f;
  }
  else {
    return easeInOut(pct);
  }
}

// ---------------------------------------------------------------------------------
// ------------------------------  Measurements  -----------------------------------
// ---------------------------------------------------------------------------------
void Measurements::add(float measurement) {
  add(Measurement{.value=measurement, .millis=millis()});
}

void Measurements::add(Measurement measurement) {
  values.push_front(measurement);
  // If the deque exceeds the specified size, remove the oldest measurement
  if (values.size() > size) {
    values.pop_back();
  }
}

Measurement Measurements::previous() {
  if (values.size() < 2) return Measurement{0.f, 0};
  return *std::next(values.begin());
}

Measurement Measurements::latest() {
  if (!values.empty()) {
    return values.front();
  }
  return Measurement{0.f, 0};
}

void Measurements::clear() {
  values.clear();  // Clear all the measurements in the deque
}

MeasurementChange Measurements::measurementChange() {
  if (values.size() < 2) return MeasurementChange{0.f, 0};
  Measurement latest = values.front();
  Measurement closestDifferent = values.back();

  for (auto it = values.begin(); it != values.end(); it = std::next(it)) {
    if (it->value != latest.value) {
      closestDifferent = *it;
      break;
    }
  }

  return MeasurementChange{
    .deltaValue = latest.value - closestDifferent.value,
    .deltaMillis=latest.millis - closestDifferent.millis
  };
}
