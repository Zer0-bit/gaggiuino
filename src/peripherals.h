#ifndef PERIPHERALS_H
#define PERIPHERALS_H

#include "pindef.h"

void  pinInit();
void  uartInit();
void  pressureSensorInit();

bool brewState();
bool steamState();

float   getPressure();
int8_t  getAdsError();

void    setBoilerOn();
void    setBoilerOff();

#endif
