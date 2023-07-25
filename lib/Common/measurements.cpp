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
  MeasurementChange result = MeasurementChange{0.f, 0};

  if (values.size() < 2) return result;

  // Use reverse iterators to more efficiently traverse the deque in reverse order
  auto reverseIterator = values.rbegin();
  auto prevRiterator = std::next(reverseIterator);

  for (; prevRiterator != values.rend(); ++reverseIterator, ++prevRiterator) {
    if (reverseIterator->value != prevRiterator->value) {
      result = MeasurementChange{
        .deltaValue = reverseIterator->value - prevRiterator->value,
        .deltaMillis = reverseIterator->millis - prevRiterator->millis,
      };
      break;
    }
  }

  return result;
}

