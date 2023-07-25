
#include "measurements.h"

void Measurements::add(float measurement) {
  add(Measurement{.value=measurement, .millis=static_cast<uint32_t>(millis())});
}

void Measurements::add(Measurement measurement) {
  values.push_front(measurement);
  // If the deque exceeds the specified size, remove the oldest measurement
  if (values.size() > size) {
    values.pop_back();
  }
}

Measurement Measurements::getPrevious() {
  if (values.size() < 2) return Measurement{0.f, 0};
  return *std::next(values.begin());
}

Measurement Measurements::getLatest() {
  if (!values.empty()) {
    return values.front();
  }
  return Measurement{0.f, 0};
}

void Measurements::clear() {
  values.clear();  // Clear all the measurements in the deque
}

/**
  * See explanation in Measurements class. This method tries to find the last 2 different
  * measurements and returns the change between them (delta and deltaTime)
  */
MeasurementChange Measurements::getMeasurementChange() {
  MeasurementChange result = MeasurementChange{ 0.f, 0 };

  if (values.size() < 2) return result;

  for (auto latest = values.begin(), prev = std::next(values.begin());
    prev != values.end();
    latest = std::next(latest), prev = std::next(prev)
  ) {
    if (latest->value != prev->value) {
      result = MeasurementChange{
        .deltaValue = latest->value - prev->value,
        .deltaMillis = latest->millis - prev->millis,
      };
      break;
    }
  }

  return result;
}
