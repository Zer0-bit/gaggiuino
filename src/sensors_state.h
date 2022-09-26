#ifndef SENSORS_STATE_H
#define SENSORS_STATE_H

struct SensorState {
  float temperature;
  float pressure;
  float puckResistance;
  bool isPressureFalling;
  bool isPressureFallingFast;
  bool isPressureRising;
  bool isPumpFlowRisingFast;
  bool isOutputFlow;
  bool isHeadSpaceFilled;
  float pumpFlow;
  float liquidPumped;
  float weightFlow;
  float weight;
};

#endif
