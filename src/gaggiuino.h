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
#include "functional/descale.h"
#include "functional/just_do_coffee.h"
#include "functional/predictive_weight.h"
#include "profiling_phases.h"
#include "peripherals/esp_comms.h"
#include "peripherals/led.h"

// Define some const values
#if defined SINGLE_BOARD
    #define GET_KTYPE_READ_EVERY    70 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#else
    #define GET_KTYPE_READ_EVERY    250 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#endif
#define GET_PRESSURE_READ_EVERY 10
#define GET_SCALES_READ_EVERY   100
#define REFRESH_SCREEN_EVERY    150 // Screen refresh interval (ms)
#define REFRESH_FLOW_EVERY      50
#define READ_TRAY_OFFSET_EVERY  1000
#define EMPTY_TRAY_WEIGHT       23456.f
#define TRAY_FULL_THRESHOLD     700.f
#define HEALTHCHECK_EVERY       30000 // system checks happen every 30sec
#define BOILER_FILL_START_TIME  3000UL // Boiler fill start time
#define BOILER_FILL_TIMEOUT     8000UL // Boiler fill timeout
#define BOILER_FILL_SKIP_TEMP   85.f // Boiler fill skip threshold

enum class OPERATION_MODES {
  OPMODE_straight9Bar,
  OPMODE_justPreinfusion,
  OPMODE_justPressureProfile,
  OPMODE_manual,
  OPMODE_preinfusionAndPressureProfile,
  OPMODE_flush,
  OPMODE_descale,
  OPMODE_empty,
  OPMODE_justFlowBasedProfiling,
  OPMODE_steam,
  OPMODE_justFlowBasedPreinfusion,
  OPMODE_everythingFlowProfiled,
  OPMODE_pressureBasedPreinfusionAndFlowProfile
} ;

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
bool preinfusionFinished = false;
bool homeScreenScalesEnabled = false;

// Other util vars
float previousSmoothedPressure;
float previousSmoothedPumpFlow;
bool startupInitFinished;

static void systemHealthCheck(float pressureThreshold);

#endif
