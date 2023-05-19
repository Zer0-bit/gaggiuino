/* 09:32 15/03/2023 - change triggering comment */
#ifndef SENSORS_STATE_H
#define SENSORS_STATE_H

struct SensorState {
  bool brewSwitchState;
  bool steamSwitchState;
  bool dreamSteamSwitchState;
  bool hotWaterSwitchState;
  bool isSteamForgottenON;
  float temperature;          // °C
  float pressure;             // bar
  float pressureChangeSpeed;  // bar/s
  float pumpFlow;             // ml/s
  float pumpFlowChangeSpeed;  // ml/s^2
  float waterPumped;
  float weightFlow;
  float weight;
  float shotWeight;
  float smoothedPressure;
  float smoothedPumpFlow;
  float smoothedWeightFlow;
  float consideredFlow;
  long pumpClicks;
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
