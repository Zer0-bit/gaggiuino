/* 09:32 15/03/2023 - change triggering comment */
#include "utils.h"
#include "math.h"

#ifndef M_PI
#define M_PI 3.14159265359
#endif

const uint8_t BUFFER_SIZE = 5;
float avgBuffer[BUFFER_SIZE] = {0.f};
int currentAvgIndex = 0;

float percentageWithTransition(float pct, TransitionCurve transition);

float mapRange(float refNumber, float refStart, float refEnd, float targetStart, float targetEnd, int decimalPrecision, TransitionCurve transition) {
  float deltaRef = refEnd - refStart;
  float deltaTarget = targetEnd - targetStart;

  if (deltaRef == 0) {
    return targetEnd;
  }

  float pct = fmax(0.0f, fmin(1.0f, abs((refNumber - refStart) / deltaRef)));

  float finalNumber = targetStart + deltaTarget * percentageWithTransition(pct, transition);

  int calcScale = (int)pow(10, decimalPrecision >= 0 ? decimalPrecision : 1);
  return (float)round(finalNumber * calcScale) / calcScale;
}

float easeIn(float pct) {
  return powf(pct, 1.675f);
}

float easeOut(float pct) {
  return 1.f - powf(1.f - pct, 1.675f);
}

float easeInOut(float pct) {
  return 0.5f * (sinf((pct - 0.5f) * (float)M_PI) + 1.f);
}

float percentageWithTransition(float pct, TransitionCurve transition) {
  if (transition == TransitionCurve::LINEAR) {
    return pct;
  }
  else if (transition == TransitionCurve::EASE_IN) {
    return easeIn(pct);
  }
  else if (transition == TransitionCurve::EASE_OUT) {
    return easeOut(pct);
  }
  else if (transition == TransitionCurve::INSTANT) {
    return 1.f;
  }
  else {
    return easeInOut(pct);
  }
}

float getAverage(float value) {
  avgBuffer[currentAvgIndex] = value;
  currentAvgIndex = (currentAvgIndex + 1) % BUFFER_SIZE;

  float sum = 0;
  for (int i = 0; i < BUFFER_SIZE; ++i) {
      sum += avgBuffer[i];
  }

  return sum / BUFFER_SIZE;
}

float truncate(float num, uint8_t precision) {
    float factor = std::pow(10.0f, precision);
    return roundf(num * factor) / factor;
}
