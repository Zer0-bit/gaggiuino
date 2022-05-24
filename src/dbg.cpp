// #include "dbg.h"

// #if defined(STM32F4xx)
// int readVref()
// {
// #ifdef __LL_ADC_CALC_VREFANALOG_VOLTAGE
//   return (__LL_ADC_CALC_VREFANALOG_VOLTAGE(analogRead(AVREF), LL_ADC_RESOLUTION));
// #else
//   return (VREFINT * ADC_RANGE / analogRead(AVREF)); // ADC sample to mV
// #endif
// }

// #ifdef ATEMP
// int readTempSensor(int32_t VRef)
// {
// #ifdef __LL_ADC_CALC_TEMPERATURE
//   return (__LL_ADC_CALC_TEMPERATURE(VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
// #elif defined(__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS)
//   return (__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(AVG_SLOPE, V25, CALX_TEMP, VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
// #else
//   return 0;
// #endif
// }
// #endif
// #endif