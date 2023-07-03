/* 09:32 15/03/2023 - change triggering comment */
#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

enum class OperationMode {
  BREW_AUTO = 0,
  BREW_MANUAL = 1,
  FLUSH = 2,
  DESCALE = 3,
  STEAM = 4,
};

struct SystemState {
  bool          startupInitFinished = false;
  OperationMode operationMode = OperationMode::BREW_AUTO;
  bool          tofReady = false;
  bool          isSteamForgottenON = false;
  bool          scalesPresent = false;
  uint32_t      timeAlive = 0; // sec
  uint8_t       descaleProgress = 0; // %
};

struct UpdateOperationMode {
  OperationMode operationMode;
};

#endif
