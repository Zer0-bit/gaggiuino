#ifndef PUMP_H
#define PUMP_H

#include <Arduino.h>

#define PUMP_RANGE 127
#define ZC_MODE    RISING

void setPumpPressure(float targetValue, float livePressure);
void setPumpOff(void);
void setPumpToRawValue(uint8_t val);

#endif
