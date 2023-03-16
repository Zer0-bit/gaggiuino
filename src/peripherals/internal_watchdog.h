/* 09:32 15/03/2023 - change triggering comment */
#ifndef INTERNALWATCHDOG_H
#define INTERNALWATCHDOG_H
#include <Arduino.h>
#include <IWatchdog.h>
#include "../lcd/lcd.h"

/*Checking whether system is booting after a hard reset initiated by the internal watchdog.*/
static inline void iwdcInit(void) {
  // IWDC init
  if(IWatchdog.isReset()) {
    lcdShowPopup("WATCHDOG RESTARTED");
    IWatchdog.clearReset();
  }
  IWatchdog.begin(3000000);
  LOG_INFO("Internal watchdog Init");
}

static inline void watchdogReload(void) {
  IWatchdog.reload();
}
#endif
