/* 09:32 15/03/2023 - change triggering comment */
#ifndef DBG_H
#define DBG_H
#if defined(STM32F4xx)
#include "stm32yyxx_ll_adc.h"
#endif

#define CALX_TEMP 25
#define V25       760
#define AVG_SLOPE 2500
#define VREFINT   1210
/* Analog read resolution */
#define LL_ADC_RESOLUTION LL_ADC_RESOLUTION_12B
#define ADC_RANGE 4096

static int readVref() {
#ifdef __LL_ADC_CALC_VREFANALOG_VOLTAGE
  return (__LL_ADC_CALC_VREFANALOG_VOLTAGE(analogRead(AVREF), LL_ADC_RESOLUTION));
#else
  return (VREFINT * ADC_RANGE / analogRead(AVREF)); // ADC sample to mV
#endif
}

static inline void dbgInit() {
  analogReadResolution(12);
}

#ifdef ATEMP
static int readTempSensor() {
  int32_t VRef = readVref();
#ifdef __LL_ADC_CALC_TEMPERATURE
  return (__LL_ADC_CALC_TEMPERATURE(VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
#elif defined(__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS)
  return (__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(AVG_SLOPE, V25, CALX_TEMP, VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
#else
  return 0;
#endif
}
#endif
#endif
