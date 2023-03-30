/* 09:32 15/03/2023 - change triggering comment */
#ifndef DESCALE_H
#define DESCALE_H

#include "../peripherals/pump.h"
#include "../lcd/lcd.h"
#include "../eeprom_data/eeprom_data.h"
#include "sensors_state.h"
#include "just_do_coffee.h"
#include <Arduino.h>

const unsigned long DESCALE_PHASE1_EVERY = 30000UL; //30000 // short pump pulses during descale
const unsigned long DESCALE_PHASE2_EVERY = 60000UL; //60000 // long pause for scale softening
const unsigned long DESCALE_PHASE3_EVERY = 10000UL; //10000 // short burst for descale efficiency

//#############################################################################################
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

enum class DescalingState {
  IDLE,
  DESCALING_PHASE1,
  DESCALING_PHASE2,
  DESCALING_PHASE3,
  FINISHED
};

void deScale(eepromValues_t &runningCfg, const SensorState &currentState);
void solenoidBeat(void);
void backFlush(const SensorState &currentState);
void flushActivated(void);
void flushDeactivated(void);
void flushPhases(void);
#endif
