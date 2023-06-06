/* 09:32 15/03/2023 - change triggering comment */
#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

struct SystemState {
  bool startupInitFinished;
  bool tofReady;
  uint16_t tofReading;
  uint8_t ledColours[3];
};

#endif
