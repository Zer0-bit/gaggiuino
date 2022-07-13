#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>

#define PUMP_RANGE 100
#define ZC_MODE    RISING

void pumpInit(int powerLineFrequency);
void setPumpPressure(float livePressure, float targetValue, float flow, bool isPressureFalling);
void setPumpOff(void);
void setPumpFullOn(void);
void setPumpToRawValue(uint8_t val);
void setPumpFlow(float liveFlow, float targetFlow, float pressureTarget);
long  getAndResetClickCounter(void);
float getPumpFlow(float cps, float pressure);
long getClicksPerSecondForFlow(float flow, float pressure);
void setPumpFlow(float flow, float pressure);
#endif
