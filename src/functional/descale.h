#ifndef DESCALE_H
#define DESCALE_H

#include <Arduino.h>
#include "../peripherals/peripherals.h"
#include "../peripherals/pump.h"
#include "../gaggiuino.h"
#include "../lcd/lcd.h"

#define DESCALE_PHASE1_EVERY    500 // short pump pulses during descale
#define DESCALE_PHASE2_EVERY    5000 // short pause for pulse effficience activation
#define DESCALE_PHASE3_EVERY    120000 // long pause for scale softening

//#############################################################################################
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

void deScale(void);

#endif
