#ifndef SENSORS_STATE_H
#define SENSORS_STATE_H

struct SensorState {
  float temperature;
  float pressure;
  float puckResistance;
  bool isPressureFalling;
  bool isPressureRising;
  bool isPumpFlowRisingFast;
  bool isOutputFlow;
  float pumpFlow;
  float liquidPumped;
  float weightFlow;
  float weight;
};

#endif
