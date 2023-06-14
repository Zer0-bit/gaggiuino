/* 09:32 15/03/2023 - change triggering comment */
#ifndef GAGGIA_UTILS_H
#define GAGGIA_UTILS_H

#include "Arduino.h"

enum class TransitionCurve {
  EASE_IN_OUT = 0,
  EASE_IN = 1,
  EASE_OUT = 2,
  LINEAR = 3,
  INSTANT = 4,
};

float mapRange(float sourceNumber, float fromA, float fromB, float toA, float toB, int decimalPrecision, TransitionCurve transition = TransitionCurve::LINEAR);

#endif
