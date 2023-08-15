/* 09:32 15/03/2023 - change triggering comment */#ifndef SCALES_H
#define SCALES_H

#include "measurements.h"
#include "gaggia_settings.h"

void scalesInit(const ScalesSettings& settings);
void scalesTare(void);
Measurement scalesGetWeight(void);
bool scalesIsPresent(void);
float scalesDripTrayWeight();
void scalesCalibrate(void);

#endif
