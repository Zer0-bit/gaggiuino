#ifndef SCALES_H
#define SCALES_H

// Needs to be here so it pleases so cppcheck is pleased.
#ifndef FORCE_PREDICTIVE_SCALES
#define FORCE_PREDICTIVE_SCALES 0
#endif

void scalesInit(float scalesF1, float scalesF2);
void scalesTare(void);
float scalesGetWeight(void);
bool scalesIsPresent(void);
float scalesDripTrayWeight();

#endif
