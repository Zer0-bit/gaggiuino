/* 09:32 15/03/2023 - change triggering comment */
#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

struct SystemState {
  bool startupInitFinished;
  bool tofReady;
  uint8_t tofReading[1000];
};

#endif
