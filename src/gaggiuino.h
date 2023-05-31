/* 09:32 15/03/2023 - change triggering comment */
#ifndef GAGGIUINO_H
#define GAGGIUINO_H

#include <Arduino.h>
#include <SimpleKalmanFilter.h>

#include "log.h"
#include "eeprom_data/eeprom_data.h"
#include "lcd/lcd.h"
#include "peripherals/internal_watchdog.h"
#include "peripherals/pump.h"
#include "peripherals/pressure_sensor.h"
#include "peripherals/scales.h"
#include "peripherals/peripherals.h"
#include "peripherals/thermocouple.h"
#include "sensors_state.h"
#include "system_state.h"
#include "functional/descale.h"
#include "functional/just_do_coffee.h"
#include "functional/predictive_weight.h"
#include "profiling_phases.h"
#include "peripherals/esp_comms.h"
#include "peripherals/led.h"
#include "peripherals/tof.h"

// Define some const values
#if defined SINGLE_BOARD
    #define GET_KTYPE_READ_EVERY    70 // max31855 amp module data read interval not recommended to be changed to lower than 70 (ms)
#else
    #define GET_KTYPE_READ_EVERY    250 // max6675 amp module data read interval not recommended to be changed to lower than 250 (ms)
#endif
#define GET_PRESSURE_READ_EVERY 10 // Pressure refresh interval (ms)
#define GET_SCALES_READ_EVERY   100 // Scales refresh interval (ms)
#define REFRESH_SCREEN_EVERY    150 // Screen refresh interval (ms)
#define REFRESH_FLOW_EVERY      50 // Flow refresh interval (ms)
#define HEALTHCHECK_EVERY       30000 // System checks happen every 30sec
#define BOILER_FILL_START_TIME  3000UL // Boiler fill start time - 3 sec since system init.
#define BOILER_FILL_TIMEOUT     8000UL // Boiler fill timeout - 8sec since system init.
#define BOILER_FILL_SKIP_TEMP   85.f // Boiler fill skip temperature threshold

enum class OPERATION_MODES {
  OPMODE_straight9Bar,
  OPMODE_justPreinfusion,
  OPMODE_justPressureProfile,
  OPMODE_manual,
  OPMODE_preinfusionAndPressureProfile,
  OPMODE_flush,
  OPMODE_descale,
  OPMODE_flowPreinfusionStraight9BarProfiling,
  OPMODE_justFlowBasedProfiling,
  OPMODE_steam,
  OPMODE_FlowBasedPreinfusionPressureBasedProfiling,
  OPMODE_everythingFlowProfiled,
  OPMODE_pressureBasedPreinfusionAndFlowProfile
} ;

//Some consts
#ifndef LEGO_VALVE_RELAY
const float calibrationPressure = 2.f;
#else
const float calibrationPressure = 0.65f;
#endif

//Timers
unsigned long systemHealthTimer;
unsigned long pageRefreshTimer;
unsigned long pressureTimer;
unsigned long brewingTimer;
unsigned long thermoTimer;
unsigned long scalesTimer;
unsigned long flowTimer;
unsigned long steamTime;

//scales vars
float previousWeight  = 0;
bool tareDone         = false;

// brew detection vars
bool brewActive = false;
bool nonBrewModeActive = false;

//PP&PI variables
int preInfusionFinishedPhaseIdx = 3;
bool homeScreenScalesEnabled = false;

// Other util vars
float previousSmoothedPressure;
float previousSmoothedPumpFlow;

static void systemHealthCheck(float pressureThreshold);

#endif
