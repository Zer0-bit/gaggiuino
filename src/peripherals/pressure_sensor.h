/* 09:32 15/03/2023 - change triggering comment */
#ifndef PRESURE_SENSSOR_H
#define PRESURE_SENSSOR_H

#include <Arduino.h>

void adsInit(void);
void i2cResetState(void);
float getPressure(void);
void getAdsError(void);

#endif
