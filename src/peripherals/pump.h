#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>
#include "sensors_state.h"

#define PUMP_RANGE 100
#define ZC_MODE    RISING

void pumpInit(int powerLineFrequency, float pumpFlowAtZero);
void setPumpPressure(float targetPressure, float flowRestriction, SensorState &currentState);
void setPumpOff(void);
void setPumpFullOn(void);
void setPumpToRawValue(uint8_t val);
long  getAndResetClickCounter(void);
float getPumpFlow(float cps, float pressure);
float getPumpFlowPerClick(float pressure);
float getClicksPerSecondForFlow(float flow, float pressure);
void setPumpFlow(float targetFlow, float pressureRestriction, SensorState &currentState);
#endif
