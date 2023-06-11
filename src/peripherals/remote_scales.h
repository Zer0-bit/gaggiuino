#ifndef REMOTE_SCALES_H
#define REMOTE_SCALES_H

#include "utils.h"

void remoteScalesTare(void);
Measurement remoteScalesGetWeight(void);
bool remoteScalesIsPresent(void);

#endif
