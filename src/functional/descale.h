#ifndef DESCALE_H
#define DESCALE_H

#include "../peripherals/pump.h"
#include "../lcd/lcd.h"
#include "../eeprom_data/eeprom_data.h"
#include "sensors_state.h"
#include "just_do_coffee.h"
#include <Arduino.h>

#define DESCALE_PHASE1_EVERY    30000 //60000 // short pump pulses during descale
#define DESCALE_PHASE2_EVERY    60000 //120000 // long pause for scale softening
#define DESCALE_PHASE3_EVERY    10000 //4000 // short burst for efficiency

//#############################################################################################
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

void deScale(eepromValues_t &runningCfg, SensorState &currentState);
void solenoidBeat(void);
void backFlush(SensorState &currentState);
void flushActivated(void);
void flushDeactivated(void);
void flushPhases(void);
#endif
