#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>

#define PUMP_RANGE 127
#define ZC_MODE    RISING

void pumpInit(int powerLineFrequency);
void setPumpPressure(float livePressure, float targetValue, float flow, bool isPressureFalling);
void setPumpOff(void);
void setPumpToRawValue(uint8_t val);
long  getAndResetClickCounter(void);
float getPumpFlow(long clickCount, float pressure);
long getClicksForFlow(float flow, float pressure);
#endif
