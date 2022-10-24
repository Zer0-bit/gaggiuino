#ifndef SENSORS_STATE_H
#define SENSORS_STATE_H

struct SensorState {
  float temperature;
  float pressure;
  float smoothedPressure;
  bool isPressureFalling;
  bool isPressureFallingFast;
  bool isPressureRising;
  bool isPressureRisingFast;
  bool isPumpFlowRisingFast;
  bool isPumpFlowFallingFast;
  float pumpFlow;
  float smoothedPumpFlow;
  float liquidPumped;
  float weightFlow;
  float weight;
  float shotWeight;
};

#endif
