#include "tof.h"
#include "measurements.h"

VL53L0X tof_sensor;
volatile uint16_t tofReading = 0;

TOF::TOF() {}

void TOF::tofTimerISR() {
  // Read TOF sensor value and update tofReading variable
  tofReading = tof_sensor.readRangeContinuousMillimeters();
}

void TOF::init(SensorState& sensor) {
  #ifdef TOF_VL53L0X
  tof_sensor.setAddress(0x29);
  tof_sensor.setMeasurementTimingBudget(200000);
  while(!sensor.tofReady) {
    sensor.tofReady = tof_sensor.init();
  }

  // Configure the hardware timer
  hw_timer = new HardwareTimer(TIM10);
  hw_timer->setOverflow(200000, MICROSEC_FORMAT);
  hw_timer->setInterruptPriority(5, 5);
  hw_timer->attachInterrupt(tofTimerISR); // Attach the ISR function to the timer


  #endif
}

uint16_t TOF::readLvl() {
  #ifdef TOF_VL53L0X
  static Measurements sensorOutput(20);
  sensorOutput.add(tofReading); // Use the stored value from the hardware timer
  uint16_t tofReadingPct = readRangeToPct(sensorOutput.latest().value);
  return tofReadingPct != 0 ? tofReadingPct : 125u;
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
