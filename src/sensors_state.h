#ifndef SENSORS_STATE_H
#define SENSORS_STATE_H

struct SensorState {
  float temperature;
  float pressure;
  float puckResistance;
  bool isPressureFalling;
  bool isPressureFallingFast;
  bool isPressureRising;
  bool isPressureRisingFast;
  bool isPumpFlowRisingFast;
  bool isOutputFlow;
  bool isHeadSpaceFilled;
  bool isPredictiveWeightForceStarted;
  float pumpFlow;
  float liquidPumped;
  float weightFlow;
  float weight;
};

#endif
