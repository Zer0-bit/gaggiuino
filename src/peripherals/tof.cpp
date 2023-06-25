#include "tof.h"
#include "measurements.h"

Adafruit_VL53L0X tof_sensor;
Measurements sensorOutput(20);

TOF::TOF() {}
TOF* TOF::instance = nullptr;

// void TOF::TimerHandler10() {
//   if (instance != nullptr && tof_sensor.isRangeComplete()) {
//     sensorOutput.add(tof_sensor.readRangeResult());
//   }
// }

void TOF::init(SensorState& sensor) {
  #ifdef TOF_VL53L0X
  while(!sensor.tofReady) {
    sensor.tofReady = tof_sensor.begin(0x29, false, &Wire, Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY);
  }
  tof_sensor.startRangeContinuous();

  // Configure the hardware timer
  // hw_timer = new HardwareTimer(TIM10);
  // hw_timer->setOverflow(1000000, MICROSEC_FORMAT);
  // hw_timer->setInterruptPriority(1, 1);
  // hw_timer->attachInterrupt(TOF::TimerHandler10); // Attach the ISR function to the timer

  instance = this;
  #endif
}

uint16_t TOF::readLvl() {
  #ifdef TOF_VL53L0X
  if(tof_sensor.isRangeComplete()) {
    sensorOutput.add(tof_sensor.readRangeResult());
  }
  return sensorOutput.latest().value != 0 ? readRangeToPct(sensorOutput.latest().value) : readRangeToPct(sensorOutput.previous().value);
  #endif
}

uint16_t TOF::readRangeToPct(uint16_t val) {
  static const std::array<uint16_t, 10> water_lvl = { 100u, 90u, 80u, 70u, 60u, 50u, 40u, 30u, 20u, 10u };
  static const std::array<uint16_t, 9> ranges = { 15u, 30u, 45u, 60u, 75u, 90u, 105u, 115u, 125u };
  for (size_t i = 0; i < ranges.size(); i++) {
    if (val <= ranges[i]) {
      return water_lvl[i];
    }
  }

  return 9u; // if val < 15
}
