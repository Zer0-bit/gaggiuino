#ifndef SCALES_H
#define SCALES_H

void scalesInit(float scalesF1, float scalesF2);
void scalesTare(void);
float scalesGetWeight(void);
bool scalesIsPresent(void);
float scalesDripTrayWeight();

#endif
