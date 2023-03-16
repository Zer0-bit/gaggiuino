/* 09:32 15/03/2023 - change triggering comment */
#ifndef GAGGIA_UTILS_H
#define GAGGIA_UTILS_H

#include "Arduino.h"

enum class TransitionCurve {
  EASE_IN_OUT,
  EASE_IN,
  EASE_OUT,
  LINEAR,
  INSTANT,
};

float mapRange(float sourceNumber, float fromA, float fromB, float toA, float toB, int decimalPrecision, TransitionCurve transition = TransitionCurve::LINEAR);
#endif
