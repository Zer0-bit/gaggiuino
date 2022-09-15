#ifndef SENSORS_STATE_H
#define SENSORS_STATE_H

struct SensorState {
  float temperature;
  float pressure;
  bool isPressureFalling;
  bool isPressureRising;
  float pumpFlow;
  float weightFlow;
  float weight;
};

#endif
