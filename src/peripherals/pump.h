/* 09:32 15/03/2023 - change triggering comment */
#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>
#include "sensors_state.h"

#define ZC_MODE    RISING

constexpr uint8_t PUMP_RANGE = 100;

void pumpInit(const int powerLineFrequency, const float pumpFlowAtZero);
void setPumpPressure(const float targetPressure, const float flowRestriction, const SensorState &currentState);
void setPumpOff(void);
void setPumpFullOn(void);
void setPumpToRawValue(const uint8_t val);
long  getAndResetClickCounter(void);
int getCPS(void);
float getPumpFlow(const float cps, const float pressure);
float getPumpFlowPerClick(const float pressure);
float getClicksPerSecondForFlow(const float flow, const float pressure);
void setPumpFlow(const float targetFlow, const float pressureRestriction, const SensorState &currentState);
#endif
