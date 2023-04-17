/* 09:32 15/03/2023 - change triggering comment */
#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

struct SystemState {
  bool startupInitFinished;
  bool pumpCalibrationFinished;
  long pumpClicks[2];
  short pumpCalibrationRetries;
  float pumpPhase_1Samples[32];
  float pumpPhase_2Samples[32];
};

#endif
