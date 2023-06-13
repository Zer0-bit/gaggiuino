
#include "measurements.h"

void Measurements::add(float measurement) {
  add(Measurement{.value=measurement, .millis=(uint32_t) millis()});
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

/**
  * See explanation in Measurements classs. This method tries to find the last 2 different
  * measurements and returns the change betweeen them (delta and deltaTime)
  */
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
