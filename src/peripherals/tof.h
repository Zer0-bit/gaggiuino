#ifndef TOF_H
#define TOF_H

#include "HardwareTimer.h"
#include <stdint.h> // for uint8_t
#include <Adafruit_VL53L0X.h>
#include <movingAvg.h>
#include "system_state.h"
#include <algorithm> // for std::generate

Adafruit_VL53L0X tof_sensor;
movingAvg mvAvg(4);

class TOF {
  public:
    TOF();
    void init(SystemState&);
    void setCustomRanges(uint16_t startValue, uint16_t endValue);
    uint16_t readLvl();
    uint16_t readRangeToPct(uint16_t);

  private:
    uint32_t tofReading;
    const std::array<uint16_t, 10> waterLvl = { 100u, 90u, 80u, 70u, 60u, 50u, 40u, 30u, 20u, 10u };
    std::array<uint16_t, 9> ranges;

    // Helper function to calculate ranges
    void calculateRanges(uint16_t startValue, uint16_t endValue);
};

TOF::TOF() {}

void TOF::init(SystemState& systemState) {
  #ifdef TOF_VL53L0X
  while(!systemState.tofReady) {
    systemState.tofReady = tof_sensor.begin(0x29, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY);
  }
  tof_sensor.startRangeContinuous();
  mvAvg.begin();
  #endif
}

void TOF::setCustomRanges(uint16_t startValue, uint16_t endValue) {
  calculateRanges(startValue, endValue);
}

void TOF::calculateRanges(uint16_t startValue, uint16_t endValue) {
  uint16_t step = (endValue - startValue) / (ranges.size() - 1);

  uint16_t current = startValue;
  std::generate(ranges.begin(), ranges.end(), [&current, step]() {
    uint16_t value = current;
    current += step;
    return value;
  });
}

uint16_t TOF::readLvl() {
  #ifdef TOF_VL53L0X
  if(tof_sensor.isRangeComplete()) {
    TOF::tofReading = mvAvg.reading(tof_sensor.readRangeResult());
  }
  #endif
  return  TOF::tofReading != 0 ? readRangeToPct(TOF::tofReading) : 30u;
}

uint16_t TOF::readRangeToPct(uint16_t val) {
  for (size_t i = 0; i < ranges.size(); i++) {
    if (val <= ranges[i]) {
      return TOF::waterLvl[i];
    }
  }

  return 9u;
}

#endif
