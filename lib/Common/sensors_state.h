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
  bool isSteamForgottenON;
  float pumpFlow;
  float smoothedPumpFlow;
  float waterPumped;
  float weightFlow;
  float weight;
  float shotWeight;
};

struct SensorStateSnapshot {
  bool brewActive;
  bool steamActive;
  float temperature;
  float pressure;
  float pumpFlow;
  float weightFlow;
  float weight;
};

#endif
