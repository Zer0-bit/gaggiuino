#include "utils.h"

float mapRange(float refNumber, float refStart, float refEnd, float targetStart, float targetEnd, int decimalPrecision) {
  float deltaRef = refEnd - refStart;
  float deltaTarget = targetEnd - targetStart;

  float pct = fmax(0.0f, fmin(1.0f, abs((refNumber - refStart) / deltaRef)));
  float finalNumber = targetStart + pct * deltaTarget;

  int calcScale = (int) pow(10, decimalPrecision);
  return (float) round(finalNumber * calcScale) / calcScale;
}
