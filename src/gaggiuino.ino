/* 09:32 15/03/2023 - change triggering comment */
#pragma GCC optimize ("Ofast")
#if defined(DEBUG_ENABLED)
  #include "dbg.h"
#endif
#include "gaggiuino.h"

SimpleKalmanFilter smoothPressure(0.6f, 0.6f, 0.1f);
SimpleKalmanFilter smoothPumpFlow(1.f, 1.f, 0.04f);
SimpleKalmanFilter smoothScalesFlow(0.5f, 0.5f, 0.01f);
SimpleKalmanFilter smoothConsideredFlow(0.1f, 0.1f, 0.1f);

//default phases. Updated in updateProfilerPhases.
Profile profile;
PhaseProfiler phaseProfiler{profile};

PredictiveWeight predictiveWeight;

SensorState currentState;

OPERATION_MODES selectedOperationalMode;

eepromValues_t runningCfg;

SystemState systemState;

LED tofnled;

void setup(void) {
  LOG_INIT();
  LOG_INFO("Gaggiuino (fw: %s) booting", AUTO_VERSION);

  // Various pins operation mode handling
  pinInit();
  LOG_INFO("Pin init");

  setBoilerOff();  // relayPin LOW
  setSteamValveRelayOff();
  setSteamBoilerRelayOff();
  LOG_INFO("Boiler turned off");

  //Pump
  setPumpOff();
  LOG_INFO("Pump turned off");

  // Valve
  closeValve();
  LOG_INFO("Valve closed");

  lcdInit();
  LOG_INFO("LCD Init");

#if defined(DEBUG_ENABLED)
  // Debug init if enabled
  dbgInit();
  LOG_INFO("DBG init");
#endif

  // Initialise comms library for talking to the ESP mcu
  espCommsInit();

  // Initialize LED
  tofnled.begin();
  tofnled.setColor(255, 255, 255); // WHITE

  // Initialising the vsaved values or writing defaults if first start
  eepromInit();
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  LOG_INFO("EEPROM Init");

  cpsInit(eepromCurrentValues);
  LOG_INFO("CPS Init");

  thermocoupleInit();
  LOG_INFO("Thermocouple Init");

  lcdUploadCfg(eepromCurrentValues);
  LOG_INFO("LCD cfg uploaded");

  adsInit();
  LOG_INFO("Pressure sensor init");

  // Scales handling
  scalesInit(eepromCurrentValues.scalesF1, eepromCurrentValues.scalesF2);
  LOG_INFO("Scales init");

  // Pump init
  pumpInit(eepromCurrentValues.powerLineFrequency, eepromCurrentValues.pumpFlowAtZero);
  LOG_INFO("Pump init");

  pageValuesRefresh(true);
  LOG_INFO("Setup sequence finished");

  // Change LED colour on setup exit.
  tofnled.setColor(255, 87, 95); // 64171

  iwdcInit();
}

//##############################################################################################################################
//############################################________________MAIN______________################################################
//##############################################################################################################################


//Main loop where all the logic is continuously run
void loop(void) {
  fillBoiler();
  pageValuesRefresh(false);
  lcdListen();
  sensorsRead();
  brewDetect();
  modeSelect();
  lcdRefresh();
  espCommsSendSensorData(currentState);
  systemHealthCheck(0.7f);
}

//##############################################################################################################################
//#############################################___________SENSORS_READ________##################################################
//##############################################################################################################################


static void sensorsRead(void) {
  sensorReadSwitches();
  espCommsReadData();
  sensorsReadTemperature();
  sensorsReadWeight();
  sensorsReadPressure();
  calculateWeightAndFlow();
  updateStartupTimer();
}

static void sensorReadSwitches(void) {
  currentState.brewSwitchState = brewState();
  currentState.steamSwitchState = steamState();
  currentState.hotWaterSwitchState = waterPinState() || (currentState.brewSwitchState && currentState.steamSwitchState); // use either an actual switch, or the GC/GCP switch combo
}

static void sensorsReadTemperature(void) {
  if (millis() > thermoTimer) {
    currentState.temperature = thermocoupleRead() - runningCfg.offsetTemp;
    thermoTimer = millis() + GET_KTYPE_READ_EVERY;
  }
}

static void sensorsReadWeight(void) {
  if (scalesIsPresent() && millis() > scalesTimer) {
    if(!tareDone) {
      scalesTare(); //Tare at the start of any weighing cycle
      if (!nonBrewModeActive && (scalesGetWeight() < -0.3f || scalesGetWeight() > 0.3f)) tareDone = false;
      else tareDone = true;
    }
    currentState.weight = scalesGetWeight();
    scalesTimer = millis() + GET_SCALES_READ_EVERY;
  }
}

static void sensorsReadPressure(void) {
  float elapsedTime = millis() - pressureTimer;

  if (elapsedTime > GET_PRESSURE_READ_EVERY) {
    float elapsedTimeSec = elapsedTime / 1000.f;
    currentState.pressure = getPressure();
    previousSmoothedPressure = currentState.smoothedPressure;
    currentState.smoothedPressure = smoothPressure.updateEstimate(currentState.pressure);
    currentState.pressureChangeSpeed = (currentState.smoothedPressure - previousSmoothedPressure) / elapsedTimeSec;
    pressureTimer = millis();
  }
}

static long sensorsReadFlow(float elapsedTimeSec) {
  long pumpClicks = getAndResetClickCounter();
  currentState.pumpClicks = (float) pumpClicks / elapsedTimeSec;

  currentState.pumpFlow = getPumpFlow(currentState.pumpClicks, currentState.smoothedPressure);

  previousSmoothedPumpFlow = currentState.smoothedPumpFlow;
  // Some flow smoothing
  currentState.smoothedPumpFlow = smoothPumpFlow.updateEstimate(currentState.pumpFlow);
  currentState.smoothedWeightFlow = currentState.smoothedPumpFlow; // use predicted flow as hw scales flow
  currentState.pumpFlowChangeSpeed = (currentState.smoothedPumpFlow - previousSmoothedPumpFlow) / elapsedTimeSec;
  return pumpClicks;
}

static void calculateWeightAndFlow(void) {
  long elapsedTime = millis() - flowTimer;

  if (brewActive) {
    // Marking for tare in case smth has gone wrong and it has exited tare already.
    if (currentState.weight < -0.3f) tareDone = false;

    if (elapsedTime > REFRESH_FLOW_EVERY) {
      flowTimer = millis();
      float elapsedTimeSec = elapsedTime / 1000.f;
      long pumpClicks = sensorsReadFlow(elapsedTimeSec);
      float consideredFlow = currentState.smoothedPumpFlow * elapsedTimeSec;
      // Update predictive class with our current phase
      CurrentPhase& phase = phaseProfiler.getCurrentPhase();
      predictiveWeight.update(currentState, phase, runningCfg);

      // Start the predictive weight calculations when conditions are true
      if (predictiveWeight.isOutputFlow() || currentState.weight > 0.4f) {
        float flowPerClick = getPumpFlowPerClick(currentState.smoothedPressure);
        float actualFlow = (consideredFlow > pumpClicks * flowPerClick) ? consideredFlow : pumpClicks * flowPerClick;
        // Probabilistically the flow is lower if the shot is just started winding up and we're flow profiling
        // if (runningCfg.flowProfileState && currentState.isPressureRising) {
        //   if (currentState.smoothedPressure < runningCfg.flowProfilePressureTarget * 0.9f) {
        //     actualFlow *= 0.6f;
        //   }
        // }
        currentState.consideredFlow = smoothConsideredFlow.updateEstimate(actualFlow);
        currentState.shotWeight = scalesIsPresent() ? currentState.weight : currentState.shotWeight + actualFlow;
      }
      currentState.waterPumped += consideredFlow;
    }
  } else {
    currentState.consideredFlow = 0.f;
    currentState.pumpClicks = getAndResetClickCounter();
    flowTimer = millis();
  }
}

//##############################################################################################################################
//############################################______PAGE_CHANGE_VALUES_REFRESH_____#############################################
//##############################################################################################################################

static void pageValuesRefresh(bool forcedUpdate) {  // Refreshing our values on page changes
  if ( lcdCurrentPageId != lcdLastCurrentPageId || forcedUpdate == true ) {
    runningCfg = lcdDownloadCfg();


    homeScreenScalesEnabled = lcdGetHomeScreenScalesEnabled();
    // MODE_SELECT should always be LAST
    selectedOperationalMode = (OPERATION_MODES) lcdGetSelectedOperationalMode();

    updateProfilerPhases();

    lcdLastCurrentPageId = lcdCurrentPageId;
  }
}

//#############################################################################################
//############################____OPERATIONAL_MODE_CONTROL____#################################
//#############################################################################################
static void modeSelect(void) {
  if (!systemState.startupInitFinished) return;

  switch (selectedOperationalMode) {
    //REPLACE ALL THE BELOW WITH OPMODE_auto_profiling
    case OPERATION_MODES::OPMODE_straight9Bar:
    case OPERATION_MODES::OPMODE_justPreinfusion:
    case OPERATION_MODES::OPMODE_justPressureProfile:
    case OPERATION_MODES::OPMODE_preinfusionAndPressureProfile:
    case OPERATION_MODES::OPMODE_flowPreinfusionStraight9BarProfiling:
    case OPERATION_MODES::OPMODE_justFlowBasedProfiling:
    case OPERATION_MODES::OPMODE_FlowBasedPreinfusionPressureBasedProfiling:
    case OPERATION_MODES::OPMODE_everythingFlowProfiled:
    case OPERATION_MODES::OPMODE_pressureBasedPreinfusionAndFlowProfile:
      nonBrewModeActive = false;
      if (currentState.hotWaterSwitchState) hotWaterMode(currentState);
      else if (currentState.steamSwitchState) steamCtrl(runningCfg, currentState);
      else {
        profiling();
        steamTime = millis();
      }
      break;
    case OPERATION_MODES::OPMODE_manual:
      nonBrewModeActive = false;
      if (!currentState.steamSwitchState) steamTime = millis();
      manualFlowControl();
      break;
    case OPERATION_MODES::OPMODE_flush:
      nonBrewModeActive = true;
      if (!currentState.steamSwitchState) steamTime = millis();
      backFlush(currentState);
      brewActive ? setBoilerOff() : justDoCoffee(runningCfg, currentState, false);
      break;
    case OPERATION_MODES::OPMODE_steam:
      nonBrewModeActive = true;
      steamCtrl(runningCfg, currentState);

      if (!currentState.steamSwitchState) {
        brewActive ? flushActivated() : flushDeactivated();
        steamCtrl(runningCfg, currentState);
        pageValuesRefresh(true);
      }
      break;
    case OPERATION_MODES::OPMODE_descale:
      nonBrewModeActive = true;
      if (!currentState.steamSwitchState) steamTime = millis();
      deScale(runningCfg, currentState);
      break;
    default:
      pageValuesRefresh(true);
      break;
  }
}

//#############################################################################################
//################################____LCD_REFRESH_CONTROL___###################################
//#############################################################################################

static void lcdRefresh(void) {

  if (millis() > pageRefreshTimer) {
    /*LCD pressure output, as a measure to beautify the graphs locking the live pressure read for the LCD alone*/
    #ifdef BEAUTIFY_GRAPH
      lcdSetPressure(currentState.smoothedPressure * 10.f);
    #else
      lcdSetPressure(
        currentState.pressure > 0.f
          ? currentState.pressure * 10.f
          : 0.f
      );
    #endif

    /*LCD temp output*/
    uint16_t brewTempSetPoint = ACTIVE_PROFILE(runningCfg).setpoint + runningCfg.offsetTemp;
    // float liveTempWithOffset = currentState.temperature - runningCfg.offsetTemp;
    uint16_t lcdTemp = ((uint16_t)currentState.temperature > ACTIVE_PROFILE(runningCfg).setpoint && currentState.brewSwitchState)
      ? (uint16_t)currentState.temperature / brewTempSetPoint + ACTIVE_PROFILE(runningCfg).setpoint
      : (uint16_t)currentState.temperature;
    lcdSetTemperature(lcdTemp);

    /*LCD weight output*/
    if (static_cast<SCREEN_MODES>(lcdCurrentPageId) == SCREEN_MODES::SCREEN_home && homeScreenScalesEnabled) {
      lcdSetWeight(currentState.weight);
    }
    else if (static_cast<SCREEN_MODES>(lcdCurrentPageId) == SCREEN_MODES::SCREEN_brew_graph
    || static_cast<SCREEN_MODES>(lcdCurrentPageId) == SCREEN_MODES::SCREEN_brew_manual) {
      if (currentState.shotWeight)
        // If the weight output is a negative value lower than -0.8 you might want to tare again before extraction starts.
        lcdSetWeight(currentState.shotWeight > -0.8f ? currentState.shotWeight : -0.9f);
    }

    /*LCD flow output*/
    // no point sending this continuously if on any other screens than brew related ones
    if ( static_cast<SCREEN_MODES>(lcdCurrentPageId) == SCREEN_MODES::SCREEN_brew_graph
      || static_cast<SCREEN_MODES>(lcdCurrentPageId) == SCREEN_MODES::SCREEN_brew_manual ) {
      lcdSetFlow(
        currentState.weight > 0.4f // currentState.weight is always zero if scales are not present
          ? currentState.smoothedWeightFlow * 10.f
          : fmaxf(currentState.consideredFlow * 100.f, currentState.smoothedPumpFlow * 10.f)
      );
    }

  #ifdef DEBUG_ENABLED
    lcdShowDebug(readTempSensor(), getAdsError());
  #endif

    /*LCD timer and warmup*/
    if (brewActive) {
      lcdSetBrewTimer((millis() > brewingTimer) ? (int)((millis() - brewingTimer) / 1000) : 0);
      lcdBrewTimerStart(); // nextion timer start
      lcdWarmupStateStop(); // Flagging warmup notification on Nextion needs to stop (if enabled)
    } else {
      lcdBrewTimerStop(); // nextion timer stop
    }

    pageRefreshTimer = millis() + REFRESH_SCREEN_EVERY;
  }
}
//#############################################################################################
//###################################____SAVE_BUTTON____#######################################
//#############################################################################################
// Save the desired temp values to EEPROM
void lcdSaveSettingsTrigger(void) {
  LOG_VERBOSE("Saving values to EEPROM");
  bool rc;
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  watchdogReload(); // reload the watchdog timer on expensive operations
  eepromValues_t lcdValues = lcdDownloadCfg(true); // :true: means read the buttons name
  watchdogReload(); // reload the watchdog timer on expensive operations

  // Target save to the currently selected profile on screen (not necessarily same as saved default)
  eepromValues_t::profile_t *eepromTargetProfile = &eepromCurrentValues.profiles[lcdValues.activeProfile];
  // Save the currently selected profile name
  snprintf(eepromTargetProfile->name, _maxProfileName, "%s", ACTIVE_PROFILE(lcdValues).name);
  eepromCurrentValues.activeProfile = lcdValues.activeProfile;

  switch (static_cast<SCREEN_MODES>(lcdCurrentPageId)){
    case SCREEN_MODES::SCREEN_brew_more:
      eepromCurrentValues.homeOnShotFinish              = lcdValues.homeOnShotFinish;
      eepromCurrentValues.basketPrefill                 = lcdValues.basketPrefill;
      eepromCurrentValues.brewDeltaState                = lcdValues.brewDeltaState;
      break;
    case SCREEN_MODES::SCREEN_brew_preinfusion: // PI Settings
      eepromTargetProfile->preinfusionState = ACTIVE_PROFILE(lcdValues).preinfusionState;
      eepromTargetProfile->preinfusionFlowState = ACTIVE_PROFILE(lcdValues).preinfusionFlowState;

      if(eepromTargetProfile->preinfusionFlowState == 0) {
        eepromTargetProfile->preinfusionSec = ACTIVE_PROFILE(lcdValues).preinfusionSec;
        eepromTargetProfile->preinfusionPressureFlowTarget = ACTIVE_PROFILE(lcdValues).preinfusionPressureFlowTarget;
        eepromTargetProfile->preinfusionBar = ACTIVE_PROFILE(lcdValues).preinfusionBar;
      }
      else {
        eepromTargetProfile->preinfusionFlowTime = ACTIVE_PROFILE(lcdValues).preinfusionFlowTime;
        eepromTargetProfile->preinfusionFlowVol = ACTIVE_PROFILE(lcdValues).preinfusionFlowVol;
        eepromTargetProfile->preinfusionFlowPressureTarget = ACTIVE_PROFILE(lcdValues).preinfusionFlowPressureTarget;
      }
      eepromTargetProfile->preinfusionFilled = ACTIVE_PROFILE(lcdValues).preinfusionFilled;
      eepromTargetProfile->preinfusionPressureAbove = ACTIVE_PROFILE(lcdValues).preinfusionPressureAbove;
      eepromTargetProfile->preinfusionWeightAbove = ACTIVE_PROFILE(lcdValues).preinfusionWeightAbove;
      break;
    case SCREEN_MODES::SCREEN_brew_soak: // Soak Settings
      eepromTargetProfile->soakState = ACTIVE_PROFILE(lcdValues).soakState;

      if(eepromTargetProfile->preinfusionFlowState == 0)
        eepromTargetProfile->soakTimePressure = ACTIVE_PROFILE(lcdValues).soakTimePressure;
      else
        eepromTargetProfile->soakTimeFlow = ACTIVE_PROFILE(lcdValues).soakTimeFlow;

      eepromTargetProfile->soakKeepPressure = ACTIVE_PROFILE(lcdValues).soakKeepPressure;
      eepromTargetProfile->soakKeepFlow = ACTIVE_PROFILE(lcdValues).soakKeepFlow;
      eepromTargetProfile->soakBelowPressure = ACTIVE_PROFILE(lcdValues).soakBelowPressure;
      eepromTargetProfile->soakAbovePressure = ACTIVE_PROFILE(lcdValues).soakAbovePressure;
      eepromTargetProfile->soakAboveWeight = ACTIVE_PROFILE(lcdValues).soakAboveWeight;
      // PI -> PF/Adv
      eepromTargetProfile->preinfusionRamp = ACTIVE_PROFILE(lcdValues).preinfusionRamp;
      eepromTargetProfile->preinfusionRampSlope = ACTIVE_PROFILE(lcdValues).preinfusionRampSlope;
      break;
    case SCREEN_MODES::SCREEN_brew_profiling: // Main Profiling Settings
      eepromTargetProfile->profilingState                = ACTIVE_PROFILE(lcdValues).profilingState;
      eepromTargetProfile->mfProfileState              = ACTIVE_PROFILE(lcdValues).mfProfileState;
      if(eepromTargetProfile->mfProfileState == 0) {
        eepromTargetProfile->mpProfilingStart            = ACTIVE_PROFILE(lcdValues).mpProfilingStart;
        eepromTargetProfile->mpProfilingFinish           = ACTIVE_PROFILE(lcdValues).mpProfilingFinish;
        eepromTargetProfile->mpProfilingSlope            = ACTIVE_PROFILE(lcdValues).mpProfilingSlope;
        eepromTargetProfile->mpProfilingSlopeShape       = ACTIVE_PROFILE(lcdValues).mpProfilingSlopeShape;
        eepromTargetProfile->mpProfilingFlowRestriction  = ACTIVE_PROFILE(lcdValues).mpProfilingFlowRestriction;
      } else {
        eepromTargetProfile->mfProfileStart                  = ACTIVE_PROFILE(lcdValues).mfProfileStart;
        eepromTargetProfile->mfProfileEnd                    = ACTIVE_PROFILE(lcdValues).mfProfileEnd;
        eepromTargetProfile->mfProfileSlope                  = ACTIVE_PROFILE(lcdValues).mfProfileSlope;
        eepromTargetProfile->mfProfileSlopeShape             = ACTIVE_PROFILE(lcdValues).mfProfileSlopeShape;
        eepromTargetProfile->mfProfilingPressureRestriction  = ACTIVE_PROFILE(lcdValues).mfProfilingPressureRestriction;
      }
      break;
case SCREEN_MODES::SCREEN_brew_transition_profile: // Advanced Profiling Settings
      eepromTargetProfile->profilingState                = ACTIVE_PROFILE(lcdValues).profilingState;
      eepromTargetProfile->tpType                        = ACTIVE_PROFILE(lcdValues).tpType;
      if(eepromTargetProfile->tpType == 0) {
        eepromTargetProfile->tpProfilingStart            = ACTIVE_PROFILE(lcdValues).tpProfilingStart;
        eepromTargetProfile->tpProfilingFinish           = ACTIVE_PROFILE(lcdValues).tpProfilingFinish;
        eepromTargetProfile->tpProfilingHold             = ACTIVE_PROFILE(lcdValues).tpProfilingHold;
        eepromTargetProfile->tpProfilingHoldLimit        = ACTIVE_PROFILE(lcdValues).tpProfilingHoldLimit;
        eepromTargetProfile->tpProfilingSlope            = ACTIVE_PROFILE(lcdValues).tpProfilingSlope;
        eepromTargetProfile->tpProfilingSlopeShape       = ACTIVE_PROFILE(lcdValues).tpProfilingSlopeShape;
        eepromTargetProfile->tpProfilingFlowRestriction  = ACTIVE_PROFILE(lcdValues).tpProfilingFlowRestriction;
      } else {
        eepromTargetProfile->tfProfileStart                  = ACTIVE_PROFILE(lcdValues).tfProfileStart;
        eepromTargetProfile->tfProfileEnd                    = ACTIVE_PROFILE(lcdValues).tfProfileEnd;
        eepromTargetProfile->tfProfileHold                   = ACTIVE_PROFILE(lcdValues).tfProfileHold;
        eepromTargetProfile->tfProfileHoldLimit              = ACTIVE_PROFILE(lcdValues).tfProfileHoldLimit;
        eepromTargetProfile->tfProfileSlope                  = ACTIVE_PROFILE(lcdValues).tfProfileSlope;
        eepromTargetProfile->tfProfileSlopeShape             = ACTIVE_PROFILE(lcdValues).tfProfileSlopeShape;
        eepromTargetProfile->tfProfilingPressureRestriction  = ACTIVE_PROFILE(lcdValues).tfProfilingPressureRestriction;
      }
      break;
    case SCREEN_MODES::SCREEN_settings_boiler:
      eepromTargetProfile->setpoint                      = ACTIVE_PROFILE(lcdValues).setpoint;
      eepromCurrentValues.steamSetPoint                 = lcdValues.steamSetPoint;
      eepromCurrentValues.offsetTemp                    = lcdValues.offsetTemp;
      eepromCurrentValues.hpwr                          = lcdValues.hpwr;
      eepromCurrentValues.mainDivider                   = lcdValues.mainDivider;
      eepromCurrentValues.brewDivider                   = lcdValues.brewDivider;
      break;
    case SCREEN_MODES::SCREEN_settings_system:
      eepromCurrentValues.warmupState                   = lcdValues.warmupState;
      eepromCurrentValues.lcdSleep                      = lcdValues.lcdSleep;
      eepromCurrentValues.scalesF1                      = lcdValues.scalesF1;
      eepromCurrentValues.scalesF2                      = lcdValues.scalesF2;
      eepromCurrentValues.pumpFlowAtZero                = lcdValues.pumpFlowAtZero;
      break;
    case SCREEN_MODES::SCREEN_shot_settings:
      eepromTargetProfile->stopOnWeightState             = ACTIVE_PROFILE(lcdValues).stopOnWeightState;
      eepromTargetProfile->shotDose                      = ACTIVE_PROFILE(lcdValues).shotDose;
      eepromTargetProfile->shotPreset                    = ACTIVE_PROFILE(lcdValues).shotPreset;
      eepromTargetProfile->shotStopOnCustomWeight        = ACTIVE_PROFILE(lcdValues).shotStopOnCustomWeight;
      break;
    default:
      break;
  }
  rc = eepromWrite(eepromCurrentValues);
  watchdogReload(); // reload the watchdog timer on expensive operations
  if (rc == true) {
    lcdShowPopup("Update successful!");
  } else {
    lcdShowPopup("Data out of range!");
  }
}

void lcdSaveProfileTrigger(void) {
  LOG_VERBOSE("Saving profile to EEPROM");
  bool rc;
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  watchdogReload(); // reload the watchdog timer on expensive operations
  eepromValues_t lcdValues = lcdDownloadCfg(true); // :true: means read the buttons name
  watchdogReload(); // reload the watchdog timer on expensive operations

  // Target save to the currently selected profile on screen (not necessarily same as saved default)
  eepromValues_t::profile_t *eepromTargetProfile = &eepromCurrentValues.profiles[lcdValues.activeProfile];

  snprintf(eepromTargetProfile->name, _maxProfileName, "%s", ACTIVE_PROFILE(lcdValues).name);
  eepromTargetProfile->preinfusionState = ACTIVE_PROFILE(lcdValues).preinfusionState;
  eepromTargetProfile->preinfusionFlowState = ACTIVE_PROFILE(lcdValues).preinfusionFlowState;

  if(eepromTargetProfile->preinfusionFlowState == 0) {
    eepromTargetProfile->preinfusionSec = ACTIVE_PROFILE(lcdValues).preinfusionSec;
    eepromTargetProfile->preinfusionPressureFlowTarget = ACTIVE_PROFILE(lcdValues).preinfusionPressureFlowTarget;
    eepromTargetProfile->preinfusionBar = ACTIVE_PROFILE(lcdValues).preinfusionBar;
  }
  else {
    eepromTargetProfile->preinfusionFlowTime = ACTIVE_PROFILE(lcdValues).preinfusionFlowTime;
    eepromTargetProfile->preinfusionFlowVol = ACTIVE_PROFILE(lcdValues).preinfusionFlowVol;
    eepromTargetProfile->preinfusionFlowPressureTarget = ACTIVE_PROFILE(lcdValues).preinfusionFlowPressureTarget;
  }
  eepromTargetProfile->preinfusionFilled = ACTIVE_PROFILE(lcdValues).preinfusionFilled;
  eepromTargetProfile->preinfusionPressureAbove = ACTIVE_PROFILE(lcdValues).preinfusionPressureAbove;
  eepromTargetProfile->preinfusionWeightAbove = ACTIVE_PROFILE(lcdValues).preinfusionWeightAbove;
  eepromTargetProfile->soakState = ACTIVE_PROFILE(lcdValues).soakState;

  if(eepromTargetProfile->preinfusionFlowState == 0)
    eepromTargetProfile->soakTimePressure = ACTIVE_PROFILE(lcdValues).soakTimePressure;
  else
    eepromTargetProfile->soakTimeFlow = ACTIVE_PROFILE(lcdValues).soakTimeFlow;

  eepromTargetProfile->soakKeepPressure = ACTIVE_PROFILE(lcdValues).soakKeepPressure;
  eepromTargetProfile->soakKeepFlow = ACTIVE_PROFILE(lcdValues).soakKeepFlow;
  eepromTargetProfile->soakBelowPressure = ACTIVE_PROFILE(lcdValues).soakBelowPressure;
  eepromTargetProfile->soakAbovePressure = ACTIVE_PROFILE(lcdValues).soakAbovePressure;
  eepromTargetProfile->soakAboveWeight = ACTIVE_PROFILE(lcdValues).soakAboveWeight;
  // PI -> PF
  eepromTargetProfile->preinfusionRamp = ACTIVE_PROFILE(lcdValues).preinfusionRamp;
  eepromTargetProfile->preinfusionRampSlope = ACTIVE_PROFILE(lcdValues).preinfusionRampSlope;
  // Main Profile PARAMS
  eepromTargetProfile->profilingState                = ACTIVE_PROFILE(lcdValues).profilingState;
  eepromTargetProfile->mfProfileState              = ACTIVE_PROFILE(lcdValues).mfProfileState;
  if(eepromTargetProfile->mfProfileState == 0) {
    eepromTargetProfile->mpProfilingStart            = ACTIVE_PROFILE(lcdValues).mpProfilingStart;
    eepromTargetProfile->mpProfilingFinish           = ACTIVE_PROFILE(lcdValues).mpProfilingFinish;
    eepromTargetProfile->mpProfilingSlope            = ACTIVE_PROFILE(lcdValues).mpProfilingSlope;
    eepromTargetProfile->mpProfilingSlopeShape       = ACTIVE_PROFILE(lcdValues).mpProfilingSlopeShape;
    eepromTargetProfile->mpProfilingFlowRestriction  = ACTIVE_PROFILE(lcdValues).mpProfilingFlowRestriction;
  } else {
    eepromTargetProfile->mfProfileStart                  = ACTIVE_PROFILE(lcdValues).mfProfileStart;
    eepromTargetProfile->mfProfileEnd                    = ACTIVE_PROFILE(lcdValues).mfProfileEnd;
    eepromTargetProfile->mfProfileSlope                  = ACTIVE_PROFILE(lcdValues).mfProfileSlope;
    eepromTargetProfile->mfProfileSlopeShape             = ACTIVE_PROFILE(lcdValues).mfProfileSlopeShape;
    eepromTargetProfile->mfProfilingPressureRestriction  = ACTIVE_PROFILE(lcdValues).mfProfilingPressureRestriction;
  }
  // Advanced/Transition Profile PARAMS
  eepromTargetProfile->tpType                        = ACTIVE_PROFILE(lcdValues).tpType;
  if(eepromTargetProfile->tpType == 0) {
    eepromTargetProfile->tpProfilingStart            = ACTIVE_PROFILE(lcdValues).tpProfilingStart;
    eepromTargetProfile->tpProfilingFinish           = ACTIVE_PROFILE(lcdValues).tpProfilingFinish;
    eepromTargetProfile->tpProfilingHold             = ACTIVE_PROFILE(lcdValues).tpProfilingHold;
    eepromTargetProfile->tpProfilingHoldLimit        = ACTIVE_PROFILE(lcdValues).tpProfilingHoldLimit;
    eepromTargetProfile->tpProfilingSlope            = ACTIVE_PROFILE(lcdValues).tpProfilingSlope;
    eepromTargetProfile->tpProfilingSlopeShape       = ACTIVE_PROFILE(lcdValues).tpProfilingSlopeShape;
    eepromTargetProfile->tpProfilingFlowRestriction  = ACTIVE_PROFILE(lcdValues).tpProfilingFlowRestriction;
  } else {
    eepromTargetProfile->tfProfileStart                  = ACTIVE_PROFILE(lcdValues).tfProfileStart;
    eepromTargetProfile->tfProfileEnd                    = ACTIVE_PROFILE(lcdValues).tfProfileEnd;
    eepromTargetProfile->tfProfileHold                   = ACTIVE_PROFILE(lcdValues).tfProfileHold;
    eepromTargetProfile->tfProfileHoldLimit              = ACTIVE_PROFILE(lcdValues).tfProfileHoldLimit;
    eepromTargetProfile->tfProfileSlope                  = ACTIVE_PROFILE(lcdValues).tfProfileSlope;
    eepromTargetProfile->tfProfileSlopeShape             = ACTIVE_PROFILE(lcdValues).tfProfileSlopeShape;
    eepromTargetProfile->tfProfilingPressureRestriction  = ACTIVE_PROFILE(lcdValues).tfProfilingPressureRestriction;
  }
  // System + Other
  eepromCurrentValues.activeProfile                 = lcdValues.activeProfile;
  eepromTargetProfile->setpoint                     = ACTIVE_PROFILE(lcdValues).setpoint;
  eepromCurrentValues.steamSetPoint                 = lcdValues.steamSetPoint;
  eepromCurrentValues.offsetTemp                    = lcdValues.offsetTemp;
  eepromCurrentValues.hpwr                          = lcdValues.hpwr;
  eepromCurrentValues.mainDivider                   = lcdValues.mainDivider;
  eepromCurrentValues.brewDivider                   = lcdValues.brewDivider;
  eepromCurrentValues.warmupState                   = lcdValues.warmupState;
  eepromCurrentValues.lcdSleep                      = lcdValues.lcdSleep;
  eepromCurrentValues.scalesF1                      = lcdValues.scalesF1;
  eepromCurrentValues.scalesF2                      = lcdValues.scalesF2;
  eepromCurrentValues.pumpFlowAtZero                = lcdValues.pumpFlowAtZero;
  eepromTargetProfile->stopOnWeightState            = ACTIVE_PROFILE(lcdValues).stopOnWeightState;
  eepromTargetProfile->shotDose                     = ACTIVE_PROFILE(lcdValues).shotDose;
  eepromTargetProfile->shotPreset                   = ACTIVE_PROFILE(lcdValues).shotPreset;
  eepromTargetProfile->shotStopOnCustomWeight       = ACTIVE_PROFILE(lcdValues).shotStopOnCustomWeight;
  eepromCurrentValues.homeOnShotFinish              = lcdValues.homeOnShotFinish;
  eepromCurrentValues.basketPrefill                 = lcdValues.basketPrefill;
  eepromCurrentValues.brewDeltaState                = lcdValues.brewDeltaState;

  rc = eepromWrite(eepromCurrentValues);
  watchdogReload(); // reload the watchdog timer on expensive operations
  if (rc == true) {
    lcdShowPopup("Update successful!");
  } else {
    lcdShowPopup("Data out of range!");
  }
}

void lcdScalesTareTrigger(void) {
  LOG_VERBOSE("Tare scales");
  if (scalesIsPresent()) scalesTare();
}

void lcdHomeScreenScalesTrigger(void) {
  LOG_VERBOSE("Scales enabled or disabled");
  homeScreenScalesEnabled = lcdGetHomeScreenScalesEnabled();
}

void lcdBrewGraphScalesTareTrigger(void) {
  LOG_VERBOSE("Predictive scales tare action completed!");
  if (!scalesIsPresent()) {
    if (currentState.shotWeight > 0.f) {
      currentState.shotWeight = 0.f;
      predictiveWeight.setIsForceStarted(true);
    } else predictiveWeight.setIsForceStarted(true);
  } else scalesTare();
}

void lcdRefreshElementsTrigger(void) {

  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  eepromValues_t lcdValues = lcdDownloadCfg();

  switch (static_cast<SCREEN_MODES>(lcdCurrentPageId)) {
    case SCREEN_MODES::SCREEN_brew_preinfusion:
      ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState = ACTIVE_PROFILE(lcdValues).preinfusionFlowState;
      break;
    case SCREEN_MODES::SCREEN_brew_profiling:
      ACTIVE_PROFILE(eepromCurrentValues).mfProfileState = ACTIVE_PROFILE(lcdValues).mfProfileState;
      break;
    default:
      lcdShowPopup("Nope!");
      break;
  }
  bool rc = eepromWrite(eepromCurrentValues);
  (rc == true) ? lcdShowPopup("Switched!") : lcdShowPopup("Fail!");

  eepromCurrentValues = eepromGetCurrentValues();
  // Make the necessary changes
  uploadPageCfg(eepromCurrentValues);
  // refresh the screen elements
  pageValuesRefresh(true);
}

void lcdQuickProfileSwitch(void) {
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();

  eepromCurrentValues.activeProfile = lcdGetSelectedProfile();
  lcdUploadProfile(eepromCurrentValues);
  lcdShowPopup("Profile switched!");
}

void lcdQuickProfilesSwitchOrSave(void) {

  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  eepromValues_t lcdValues = lcdDownloadCfg();

  switch (static_cast<SCREEN_MODES>(lcdCurrentPageId)) {
    case SCREEN_MODES::SCREEN_brew_preinfusion:
      ACTIVE_PROFILE(eepromCurrentValues).preinfusionFlowState = ACTIVE_PROFILE(lcdValues).preinfusionFlowState;
      break;
    case SCREEN_MODES::SCREEN_brew_profiling:
      ACTIVE_PROFILE(eepromCurrentValues).mfProfileState = ACTIVE_PROFILE(lcdValues).mfProfileState;
      break;
    default:
      lcdShowPopup("Nope!");
      break;
  }
  bool rc = eepromWrite(eepromCurrentValues);
  (rc == true) ? lcdShowPopup("Switched!") : lcdShowPopup("Fail!");

  eepromCurrentValues = eepromGetCurrentValues();
  // Make the necessary changes
  uploadPageCfg(eepromCurrentValues);
  // refresh the screen elements
  pageValuesRefresh(true);
}

//#############################################################################################
//###############################____PRESSURE_CONTROL____######################################
//#############################################################################################
static void updateProfilerPhases(void) {
  float shotTarget = -1.f;
  float isPressureAbove = -1.f;
  float isWeightAbove = -1.f;
  float isPressureBelow = -1.f;
  float isWaterPumped = -1.f;

  if (ACTIVE_PROFILE(runningCfg).stopOnWeightState) {
    shotTarget = (ACTIVE_PROFILE(runningCfg).shotStopOnCustomWeight < 1.f)
      ? ACTIVE_PROFILE(runningCfg).shotDose * ACTIVE_PROFILE(runningCfg).shotPreset
      : ACTIVE_PROFILE(runningCfg).shotStopOnCustomWeight;
  }


  //update global stop conditions (currently only stopOnWeight is configured in nextion)
  profile.globalStopConditions = GlobalStopConditions{ .weight=shotTarget };

  profile.clear();
  //Setup release pressure + fill@4ml/sec
  if (runningCfg.basketPrefill) {
    addFillBasketPhase(4.5f);
  }

  // Setup pre-infusion if needed
  if (ACTIVE_PROFILE(runningCfg).preinfusionState) {
    if (ACTIVE_PROFILE(runningCfg).preinfusionFlowState) { // flow based PI enabled
      isPressureAbove = ACTIVE_PROFILE(runningCfg).preinfusionPressureAbove ? ACTIVE_PROFILE(runningCfg).preinfusionFlowPressureTarget : -1.f;
      isWeightAbove = ACTIVE_PROFILE(runningCfg).preinfusionWeightAbove > 0.f ? ACTIVE_PROFILE(runningCfg).preinfusionWeightAbove : -1.f;
      isWaterPumped = ACTIVE_PROFILE(runningCfg).preinfusionFilled > 0.f ? ACTIVE_PROFILE(runningCfg).preinfusionFilled : -1.f;

      addFlowPhase(Transition{ACTIVE_PROFILE(runningCfg).preinfusionFlowVol}, ACTIVE_PROFILE(runningCfg).preinfusionFlowPressureTarget, ACTIVE_PROFILE(runningCfg).preinfusionFlowTime * 1000, isPressureAbove, -1, isWeightAbove, isWaterPumped);
    } else { // pressure based PI enabled
      // For now handling phase switching on restrictions here but as this grow will have to deal with it otherwise.
      isPressureAbove = ACTIVE_PROFILE(runningCfg).preinfusionPressureAbove ? ACTIVE_PROFILE(runningCfg).preinfusionBar : -1.f;
      isWeightAbove = ACTIVE_PROFILE(runningCfg).preinfusionWeightAbove > 0.f ? ACTIVE_PROFILE(runningCfg).preinfusionWeightAbove : -1.f;
      isWaterPumped = ACTIVE_PROFILE(runningCfg).preinfusionFilled > 0.f ? ACTIVE_PROFILE(runningCfg).preinfusionFilled : -1.f;

      addPressurePhase(Transition{ACTIVE_PROFILE(runningCfg).preinfusionBar}, ACTIVE_PROFILE(runningCfg).preinfusionPressureFlowTarget, ACTIVE_PROFILE(runningCfg).preinfusionSec * 1000, isPressureAbove, -1, isWeightAbove, isWaterPumped);
    }
  }
  // Setup the soak phase if neecessary
  if (ACTIVE_PROFILE(runningCfg).soakState) {
    uint16_t phaseSoak = ACTIVE_PROFILE(runningCfg).preinfusionFlowState ? ACTIVE_PROFILE(runningCfg).soakTimeFlow : ACTIVE_PROFILE(runningCfg).soakTimePressure;
    float maintainFlow = ACTIVE_PROFILE(runningCfg).soakKeepFlow > 0.f ? ACTIVE_PROFILE(runningCfg).soakKeepFlow : -1.f;
    float maintainPressure = ACTIVE_PROFILE(runningCfg).soakKeepPressure > 0.f ? ACTIVE_PROFILE(runningCfg).soakKeepPressure : -1.f;
    isPressureBelow = ACTIVE_PROFILE(runningCfg).soakBelowPressure > 0.f ? ACTIVE_PROFILE(runningCfg).soakBelowPressure : -1.f;
    isPressureAbove = ACTIVE_PROFILE(runningCfg).soakAbovePressure > 0.f ? ACTIVE_PROFILE(runningCfg).soakAbovePressure : -1.f;
    isWeightAbove = ACTIVE_PROFILE(runningCfg).soakAboveWeight > 0.f ? ACTIVE_PROFILE(runningCfg).soakAboveWeight : -1.f;

    if (maintainPressure > 0.f)
      addPressurePhase(Transition{maintainPressure}, (maintainFlow > 0.f ? maintainFlow : 2.5f), phaseSoak * 1000, isPressureAbove, isPressureBelow, isWeightAbove, -1);
    else if(maintainFlow > 0.f)
      addFlowPhase(Transition{maintainFlow},  -1, phaseSoak * 1000, isPressureAbove, isPressureBelow, isWeightAbove, -1);
    else
      addPressurePhase(Transition{maintainPressure}, maintainFlow, phaseSoak * 1000, isPressureAbove, isPressureBelow, isWeightAbove, -1);
  }
  preInfusionFinishedPhaseIdx = profile.phaseCount();

  // Setup ramp phase if necessary
  if (ACTIVE_PROFILE(runningCfg).preinfusionRamp > 0) {
    if (ACTIVE_PROFILE(runningCfg).mfProfileState) {
      float holdLimit = -1.f; //ACTIVE_PROFILE(runningCfg).tfProfileHoldLimit > 0.f ? ACTIVE_PROFILE(runningCfg).tfProfileHoldLimit : -1;

      addFlowPhase(Transition{ACTIVE_PROFILE(runningCfg).mfProfileStart, (TransitionCurve)ACTIVE_PROFILE(runningCfg).preinfusionRampSlope}, holdLimit, ACTIVE_PROFILE(runningCfg).preinfusionRamp * 1000, -1, -1, -1, -1);
      // addPressurePhase(Transition{ holdLimit, (TransitionCurve)ACTIVE_PROFILE(runningCfg).preinfusionRampSlope }, ACTIVE_PROFILE(runningCfg).flowProfileStart, ACTIVE_PROFILE(runningCfg).preinfusionRamp * 1000, -1, -1, -1, -1);
    } else {
      float holdLimit = -1.f; //ACTIVE_PROFILE(runningCfg).flowProfileHoldLimit > 0.f ? ACTIVE_PROFILE(runningCfg).flowProfileHoldLimit : -1;

      addPressurePhase(Transition{ACTIVE_PROFILE(runningCfg).mpProfilingStart, (TransitionCurve)ACTIVE_PROFILE(runningCfg).preinfusionRampSlope}, holdLimit, ACTIVE_PROFILE(runningCfg).preinfusionRamp * 1000, -1, -1, -1, -1);
    }
  }

  // Setup shot profiling
  if (ACTIVE_PROFILE(runningCfg).profilingState) {
    // ----------------- Transition Profile ----------------- //
    if (ACTIVE_PROFILE(runningCfg).tpType) { // flow based profiling enabled
      /* Setting the phase specific restrictions */
      /* ------------------------------------------ */
      float fpStart = ACTIVE_PROFILE(runningCfg).tfProfileStart;
      float fpEnd = ACTIVE_PROFILE(runningCfg).tfProfileEnd;
      uint16_t fpHold = ACTIVE_PROFILE(runningCfg).tfProfileHold;
      float holdLimit = ACTIVE_PROFILE(runningCfg).tfProfileHoldLimit > 0.f ? ACTIVE_PROFILE(runningCfg).tfProfileHoldLimit : -1;
      /* ------------------------------------------ */

      if (fpStart > 0.f && fpHold > 0) {
        addFlowPhase(Transition{ fpStart }, holdLimit, fpHold * 1000, -1, -1, -1, -1);
      }
      addFlowPhase(Transition{ fpEnd, (TransitionCurve)ACTIVE_PROFILE(runningCfg).tfProfileSlopeShape, ACTIVE_PROFILE(runningCfg).tfProfileSlope * 1000 }, ACTIVE_PROFILE(runningCfg).tfProfilingPressureRestriction, -1, -1, -1, -1, -1);
    } else { // pressure based profiling enabled
    /* Setting the phase specific restrictions */
    /* ------------------------------------------ */
      float ppStart = ACTIVE_PROFILE(runningCfg).tpProfilingStart;
      float ppEnd = ACTIVE_PROFILE(runningCfg).tpProfilingFinish;
      uint16_t ppHold = ACTIVE_PROFILE(runningCfg).tpProfilingHold;
      float holdLimit = ACTIVE_PROFILE(runningCfg).tpProfilingHoldLimit > 0.f ? ACTIVE_PROFILE(runningCfg).tpProfilingHoldLimit : -1;
    /* ------------------------------------------ */
      if (ppStart > 0.f && ppHold > 0) {
        addPressurePhase(Transition{ppStart}, holdLimit, ppHold * 1000, -1, -1, -1, -1);
      }
      addPressurePhase(Transition{ppStart, ppEnd, (TransitionCurve)ACTIVE_PROFILE(runningCfg).tpProfilingSlopeShape, ACTIVE_PROFILE(runningCfg).tpProfilingSlope * 1000}, ACTIVE_PROFILE(runningCfg).tpProfilingFlowRestriction, -1, -1, -1, -1, -1);
    }

    // ----------------- Main Profile ----------------- //
    if (ACTIVE_PROFILE(runningCfg).mfProfileState) { // flow based profiling enabled
      /* Setting the phase specific restrictions */
      /* ------------------------------------------ */
      float fpStart = ACTIVE_PROFILE(runningCfg).mfProfileStart;
      float fpEnd = ACTIVE_PROFILE(runningCfg).mfProfileEnd;
      /* ------------------------------------------ */
      addFlowPhase(Transition{ fpStart, fpEnd, (TransitionCurve)ACTIVE_PROFILE(runningCfg).mfProfileSlopeShape, ACTIVE_PROFILE(runningCfg).mfProfileSlope * 1000 }, ACTIVE_PROFILE(runningCfg).mfProfilingPressureRestriction, -1, -1, -1, -1, -1);
    } else { // pressure based profiling enabled
    /* Setting the phase specific restrictions */
    /* ------------------------------------------ */
      float ppStart = ACTIVE_PROFILE(runningCfg).mpProfilingStart;
      float ppEnd = ACTIVE_PROFILE(runningCfg).mpProfilingFinish;
    /* ------------------------------------------ */
      addPressurePhase(Transition{ppStart, ppEnd, (TransitionCurve)ACTIVE_PROFILE(runningCfg).mpProfilingSlopeShape, ACTIVE_PROFILE(runningCfg).mpProfilingSlope * 1000}, ACTIVE_PROFILE(runningCfg).mpProfilingFlowRestriction, -1, -1, -1, -1, -1);
    }
  } else { // Shot profiling disabled. Default to 9 bars
    addPressurePhase(Transition{9.f, (TransitionCurve)ACTIVE_PROFILE(runningCfg).preinfusionRampSlope, ACTIVE_PROFILE(runningCfg).preinfusionRamp * 1000}, -1, -1, -1, -1, -1, -1);
  }
}

void addFillBasketPhase(float flowRate) {
  addFlowPhase(Transition(flowRate), -1, -1, 0.1f, -1, -1, -1);
}

void addPressurePhase(Transition pressure, float flowRestriction, int timeMs, float pressureAbove, float pressureBelow, float shotWeight, float isWaterPumped) {
  addPhase(PHASE_TYPE::PHASE_TYPE_PRESSURE, pressure, flowRestriction, timeMs, pressureAbove, pressureBelow, shotWeight, isWaterPumped);
}

void addFlowPhase(Transition flow, float pressureRestriction, int timeMs, float pressureAbove, float pressureBelow, float shotWeight, float isWaterPumped) {
  addPhase(PHASE_TYPE::PHASE_TYPE_FLOW, flow, pressureRestriction, timeMs, pressureAbove, pressureBelow, shotWeight, isWaterPumped);
}

void addPhase(PHASE_TYPE type, Transition target, float restriction, int timeMs, float pressureAbove, float pressureBelow, float shotWeight, float isWaterPumped) {
  profile.addPhase(Phase {
    .type           = type,
    .target         = target,
    .restriction    = restriction,
    .stopConditions = PhaseStopConditions{ .time=timeMs, .pressureAbove=pressureAbove, .pressureBelow=pressureBelow, .weight=shotWeight, .waterPumpedInPhase=isWaterPumped }
  });
}

void onProfileReceived(Profile& newProfile) {
}

static void profiling(void) {
  if (brewActive) { //runs this only when brew button activated and pressure profile selected
    uint32_t timeInShot = millis() - brewingTimer;
    phaseProfiler.updatePhase(timeInShot, currentState, runningCfg);
    CurrentPhase& currentPhase = phaseProfiler.getCurrentPhase();
    ShotSnapshot shotSnapshot = buildShotSnapshot(timeInShot, currentState, currentPhase);
    espCommsSendShotData(shotSnapshot, 100);

    if (phaseProfiler.isFinished()) {
      setPumpOff();
      closeValve();
      brewActive = false;
    } else if (currentPhase.getType() == PHASE_TYPE::PHASE_TYPE_PRESSURE) {
      float newBarValue = currentPhase.getTarget();
      float flowRestriction =  currentPhase.getRestriction();
      openValve();
      setPumpPressure(newBarValue, flowRestriction, currentState);
    } else {
      float newFlowValue = currentPhase.getTarget();
      float pressureRestriction =  currentPhase.getRestriction();
      openValve();
      setPumpFlow(newFlowValue, pressureRestriction, currentState);
    }
  } else {
    setPumpOff();
    closeValve();
  }
  // Keep that water at temp
  justDoCoffee(runningCfg, currentState, brewActive);
}

static void manualFlowControl(void) {
  if (brewActive) {
    openValve();
    float flow_reading = lcdGetManualFlowVol() / 10.f ;
    setPumpFlow(flow_reading, 0.f, currentState);
  } else {
    setPumpOff();
    closeValve();
  }
  justDoCoffee(runningCfg, currentState, brewActive);
}

//#############################################################################################
//###################################____BREW DETECT____#######################################
//#############################################################################################

static void brewDetect(void) {
  // Do not allow brew detection while system hasn't finished it's startup procedures.
  if (!systemState.startupInitFinished) {
    return;
  }

  static bool paramsReset = true;

  if (currentState.brewSwitchState) {
    if(!paramsReset) {
      lcdWakeUp();
      brewParamsReset();
      paramsReset = true;
      brewActive = true;
    }
    // needs to be here as it creates a locking state soemtimes if not kept up to date during brew
    // mainly when shotWeight restriction kick in.
    systemHealthTimer = millis() + HEALTHCHECK_EVERY;
  } else {
    brewActive = false;
    currentState.pumpClicks = getAndResetClickCounter();
    if(paramsReset) {
      brewParamsReset();
      paramsReset = false;
    }
  }
}

static void brewParamsReset(void) {
  tareDone                 = false;
  currentState.shotWeight  = 0.f;
  currentState.pumpFlow    = 0.f;
  previousWeight           = 0.f;
  currentState.weight      = 0.f;
  currentState.waterPumped = 0.f;
  brewingTimer             = millis();
  flowTimer                = brewingTimer;
  systemHealthTimer        = brewingTimer + HEALTHCHECK_EVERY;

  predictiveWeight.reset();
  phaseProfiler.reset();
}

static inline void systemHealthCheck(float pressureThreshold) {
  //Reloading the watchdog timer, if this function fails to run MCU is rebooted
  watchdogReload();

  /* This *while* is here to prevent situations where the system failed to get a temp reading and temp reads as 0 or -7(cause of the offset)
  If we would use a non blocking function then the system would keep the SSR in HIGH mode which would most definitely cause boiler overheating */
  while (currentState.temperature <= 0.0f || currentState.temperature == NAN || currentState.temperature >= 170.0f) {
    //Reloading the watchdog timer, if this function fails to run MCU is rebooted
    watchdogReload();
    /* In the event of the temp failing to read while the SSR is HIGH
    we force set it to LOW while trying to get a temp reading - IMPORTANT safety feature */
    setPumpOff();
    setBoilerOff();
    setSteamBoilerRelayOff();
    if (millis() > thermoTimer) {
      LOG_ERROR("Cannot read temp from thermocouple (last read: %.1lf)!", static_cast<double>(currentState.temperature));
      currentState.steamSwitchState ? lcdShowPopup("COOLDOWN") : lcdShowPopup("TEMP READ ERROR"); // writing a LCD message
      currentState.temperature  = thermocoupleRead() - runningCfg.offsetTemp;  // Making sure we're getting a value
      thermoTimer = millis() + GET_KTYPE_READ_EVERY;
    }
  }

  /*Shut down heaters if steam has been ON and unused fpr more than 10 minutes.*/
  while (currentState.isSteamForgottenON) {
    //Reloading the watchdog timer, if this function fails to run MCU is rebooted
    watchdogReload();
    lcdShowPopup("TURN STEAM OFF NOW!");
    setPumpOff();
    setBoilerOff();
    setSteamBoilerRelayOff();
    currentState.isSteamForgottenON = currentState.steamSwitchState;
  }

  //Releasing the excess pressure after steaming or brewing if necessary
  #if defined LEGO_VALVE_RELAY || defined SINGLE_BOARD

  // No point going through the whole thing if this first condition isn't met.
  if (currentState.brewSwitchState || currentState.steamSwitchState || currentState.hotWaterSwitchState) {
    systemHealthTimer = millis() + HEALTHCHECK_EVERY;
    return;
  }
  // Should enter the block every "systemHealthTimer" seconds
  if (millis() >= systemHealthTimer) {
    while (currentState.smoothedPressure >= pressureThreshold && currentState.temperature < 100.f)
    {
      //Reloading the watchdog timer, if this function fails to run MCU is rebooted
      watchdogReload();
      switch (static_cast<SCREEN_MODES>(lcdCurrentPageId)) {
        case SCREEN_MODES::SCREEN_brew_manual:
        case SCREEN_MODES::SCREEN_brew_graph:
        case SCREEN_MODES::SCREEN_graph_preview:
          brewDetect();
          lcdRefresh();
          lcdListen();
          sensorsRead();
          justDoCoffee(runningCfg, currentState, brewActive);
          break;
        default:
          sensorsRead();
          lcdShowPopup("Releasing pressure!");
          setPumpOff();
          setBoilerOff();
          setSteamValveRelayOff();
          setSteamBoilerRelayOff();
          openValve();
          break;
      }
    }
    closeValve();
    systemHealthTimer = millis() + HEALTHCHECK_EVERY;
  }
  // Throwing a pressure release countodown.
  if (static_cast<SCREEN_MODES>(lcdCurrentPageId) == SCREEN_MODES::SCREEN_brew_graph) return;
  if (static_cast<SCREEN_MODES>(lcdCurrentPageId) == SCREEN_MODES::SCREEN_brew_manual) return;

  if (currentState.smoothedPressure >= pressureThreshold && currentState.temperature < 100.f) {
    if (millis() >= systemHealthTimer - 3500ul && millis() <= systemHealthTimer - 500ul) {
      char tmp[25];
      int countdown = (int)(systemHealthTimer-millis())/1000;
      unsigned int check = snprintf(tmp, sizeof(tmp), "Dropping beats in: %i", countdown);
      if (check > 0 && check <= sizeof(tmp)) {
        lcdShowPopup(tmp);
      }
    }
  }
  #endif
}

// Function to track time since system has started
static unsigned long getTimeSinceInit(void) {
  static unsigned long startTime = millis();
  return millis() - startTime;
}

static void fillBoiler(void) {
  #if defined LEGO_VALVE_RELAY || defined SINGLE_BOARD

  if (systemState.startupInitFinished) {
    return;
  }

  if (currentState.temperature > BOILER_FILL_SKIP_TEMP) {
    systemState.startupInitFinished = true;
    return;
  }

  if (isBoilerFillPhase(getTimeSinceInit()) && !isSwitchOn()) {
    fillBoilerUntilThreshod(getTimeSinceInit());
  }
  else if (isSwitchOn()) {
    lcdShowPopup("Brew Switch ON!");
  }
#else
  systemState.startupInitFinished = true;
#endif
}

static bool isBoilerFillPhase(unsigned long elapsedTime) {
  return static_cast<SCREEN_MODES>(lcdCurrentPageId) == SCREEN_MODES::SCREEN_home && elapsedTime >= BOILER_FILL_START_TIME;
}

static bool isBoilerFull(unsigned long elapsedTime) {
  bool boilerFull = false;
  if (elapsedTime > BOILER_FILL_START_TIME + 1000UL) {
    boilerFull =  (previousSmoothedPressure - currentState.smoothedPressure > -0.02f)
                &&
                  (previousSmoothedPressure - currentState.smoothedPressure < 0.001f);
  }

  return elapsedTime >= BOILER_FILL_TIMEOUT || boilerFull;
}

// Checks if Brew switch is ON
static bool isSwitchOn(void) {
  return currentState.brewSwitchState && static_cast<SCREEN_MODES>(lcdCurrentPageId) == SCREEN_MODES::SCREEN_home;
}

static void fillBoilerUntilThreshod(unsigned long elapsedTime) {
  if (elapsedTime >= BOILER_FILL_TIMEOUT) {
    systemState.startupInitFinished = true;
    return;
  }

  if (isBoilerFull(elapsedTime)) {
    closeValve();
    setPumpOff();
    systemState.startupInitFinished = true;
    return;
  }

  lcdShowPopup("Filling boiler!");
  openValve();
  setPumpToRawValue(35);
}

static void updateStartupTimer(void) {
  lcdSetUpTime(getTimeSinceInit() / 1000);
}

static void cpsInit(eepromValues_t &eepromValues) {
  int cps = getCPS();
  if (cps > 110) { // double 60 Hz
    eepromValues.powerLineFrequency = 60u;
  } else if (cps > 80) { // double 50 Hz
    eepromValues.powerLineFrequency = 50u;
  } else if (cps > 55) { // 60 Hz
    eepromValues.powerLineFrequency = 60u;
  } else if (cps > 0) { // 50 Hz
    eepromValues.powerLineFrequency = 50u;
  }
}
