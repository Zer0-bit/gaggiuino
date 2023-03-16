/* 09:32 15/03/2023 - change triggering comment */
#ifndef JUST_DO_COFFEE_H
#define JUST_DO_COFFEE_H

#include "utils.h"
#include "../peripherals/peripherals.h"
#include "../peripherals/pump.h"
#include "../eeprom_data/eeprom_data.h"
#include "sensors_state.h"
#include <Arduino.h>


const uint32_t STEAM_TIMEOUT = 900000UL;
const float MAX_WATER_TEMP = 105.f;

void justDoCoffee(const eepromValues_t &runningCfg, const SensorState &currentState, const bool brewActive, const bool preinfusionFinished);
void pulseHeaters(const uint32_t pulseLength, const int factor_1, const int factor_2, const bool brewActive);
void steamCtrl(const eepromValues_t &runningCfg, SensorState &currentState);
void hotWaterMode(const SensorState &currentState);

#endif

