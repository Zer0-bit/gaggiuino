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
  calibratePump();
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
    currentState.temperature = thermocouple.readCelsius() - runningCfg.offsetTemp;
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
  if (millis() > pressureTimer) {
    previousSmoothedPressure = currentState.smoothedPressure;
    currentState.pressure = getPressure();
    currentState.smoothedPressure = smoothPressure.updateEstimate(currentState.pressure);
    currentState.isPressureRising = currentState.smoothedPressure >= previousSmoothedPressure + 0.01f;
    currentState.isPressureRisingFast = currentState.smoothedPressure >= previousSmoothedPressure + 0.05f;
    currentState.isPressureFalling = currentState.smoothedPressure <= previousSmoothedPressure - 0.005f;
    currentState.isPressureFallingFast = currentState.smoothedPressure <= previousSmoothedPressure - 0.01f;
    currentState.isPressureMaxed = currentState.smoothedPressure >= runningCfg.pressureProfilingState ? runningCfg.pressureProfilingStart - 0.5f : runningCfg.flowProfilePressureTarget - 0.5f;

    pressureTimer = millis() + GET_PRESSURE_READ_EVERY;
  }
}

static long sensorsReadFlow(float elapsedTime) {
  long pumpClicks = getAndResetClickCounter();
  currentState.pumpClicks = 1000.f * (float)pumpClicks / elapsedTime;

  currentState.pumpFlow = getPumpFlow(currentState.pumpClicks, currentState.smoothedPressure);

  previousSmoothedPumpFlow = currentState.smoothedPumpFlow;
  // Some flow smoothing
  currentState.smoothedPumpFlow = smoothPumpFlow.updateEstimate(currentState.pumpFlow);
  currentState.smoothedWeightFlow = currentState.smoothedPumpFlow; // use predicted flow as hw scales flow
  return pumpClicks;
}

static void calculateWeightAndFlow(void) {
  long elapsedTime = millis() - flowTimer;

  if (brewActive) {
    // Marking for tare in case smth has gone wrong and it has exited tare already.
    if (currentState.weight < -0.3f) tareDone = false;

    if (elapsedTime > REFRESH_FLOW_EVERY) {
      flowTimer = millis();
      long pumpClicks = sensorsReadFlow(elapsedTime);
      float consideredFlow = currentState.smoothedPumpFlow * (float)elapsedTime / 1000.f;
      // Some helper vars
      currentState.isPumpFlowRisingFast = currentState.smoothedPumpFlow > previousSmoothedPumpFlow + 0.1f;
      currentState.isPumpFlowFallingFast = currentState.smoothedPumpFlow < previousSmoothedPumpFlow - 0.1f;
      // Update predictive class with our current phase
      CurrentPhase& phase = phaseProfiler.getCurrentPhase();
      predictiveWeight.update(currentState, phase, runningCfg);

      // Start the predictive weight calculations when conditions are true
      if (predictiveWeight.isOutputFlow() || currentState.weight > 0.4f) {
        float flowPerClick = getPumpFlowPerClick(currentState.smoothedPressure);
        float actualFlow = (consideredFlow > pumpClicks * flowPerClick) ? consideredFlow : pumpClicks * flowPerClick;
        // Probabilistically the flow is lower if the shot is just started winding up and we're flow profiling
        if (runningCfg.flowProfileState && currentState.isPressureRising
        && currentState.smoothedPressure < runningCfg.flowProfilePressureTarget * 0.9f) {
          actualFlow *= 0.6f;
        }
        // For cases where pump flow ends up being lower than hw scales flow.
        // if (scalesIsPresent()) {
        //   currentState.weightFlow = fmaxf(0.f, (currentState.shotWeight - previousWeight) * 1000.f / (float)elapsedTime);
        //   // currentState.smoothedWeightFlow = fmaxf(smoothScalesFlow.updateEstimate(currentState.weightFlow), actualFlow);
        //   currentState.smoothedWeightFlow = actualFlow;
        //   previousWeight = currentState.shotWeight;
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
    case OPERATION_MODES::OPMODE_justFlowBasedProfiling:
    case OPERATION_MODES::OPMODE_justFlowBasedPreinfusion:
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
      brewActive ? setBoilerOff() : justDoCoffee(runningCfg, currentState, false, preinfusionFinished);
      break;
    case OPERATION_MODES::OPMODE_steam:
      nonBrewModeActive = true;
      if (!currentState.steamSwitchState) {
        brewActive ? flushActivated() : flushDeactivated();
        steamTime = millis();
      } else {
        steamCtrl(runningCfg, currentState);
      }
      break;
    case OPERATION_MODES::OPMODE_descale:
      nonBrewModeActive = true;
      if (!currentState.steamSwitchState) steamTime = millis();
      deScale(runningCfg, currentState);
      break;
    case OPERATION_MODES::OPMODE_empty:
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
    uint16_t lcdTemp = ((uint16_t)currentState.temperature > runningCfg.setpoint - runningCfg.offsetTemp && currentState.brewSwitchState)
      ? (uint16_t)currentState.temperature / (runningCfg.setpoint - runningCfg.offsetTemp) + (runningCfg.setpoint - runningCfg.offsetTemp)
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
  eepromValues_t lcdValues = lcdDownloadCfg();

  switch (static_cast<SCREEN_MODES>(lcdCurrentPageId)){
    case SCREEN_MODES::SCREEN_brew_settings:
      eepromCurrentValues.homeOnShotFinish              = lcdValues.homeOnShotFinish;
      eepromCurrentValues.basketPrefill                 = lcdValues.basketPrefill;
      eepromCurrentValues.brewDeltaState                = lcdValues.brewDeltaState;
      eepromCurrentValues.warmupState                   = lcdValues.warmupState;
      eepromCurrentValues.switchPhaseOnThreshold        = lcdValues.switchPhaseOnThreshold;
      eepromCurrentValues.switchPhaseOnPressureBelow    = lcdValues.switchPhaseOnPressureBelow;
      eepromCurrentValues.switchOnWeightAbove           = lcdValues.switchOnWeightAbove;
      eepromCurrentValues.switchOnWaterPumped           = lcdValues.switchOnWaterPumped;
      break;
    case SCREEN_MODES::SCREEN_profiles:
      // PRESSURE PARAMS
      eepromCurrentValues.pressureProfilingStart        = lcdValues.pressureProfilingStart;
      eepromCurrentValues.pressureProfilingFinish       = lcdValues.pressureProfilingFinish;
      eepromCurrentValues.pressureProfilingHold         = lcdValues.pressureProfilingHold;
      eepromCurrentValues.pressureProfilingLength       = lcdValues.pressureProfilingLength;
      eepromCurrentValues.pressureProfilingState        = lcdValues.pressureProfilingState;
      eepromCurrentValues.preinfusionState              = lcdValues.preinfusionState;
      eepromCurrentValues.preinfusionSec                = lcdValues.preinfusionSec;
      eepromCurrentValues.preinfusionBar                = lcdValues.preinfusionBar;
      eepromCurrentValues.preinfusionSoak               = lcdValues.preinfusionSoak;
      eepromCurrentValues.preinfusionRamp               = lcdValues.preinfusionRamp;

      // FLOW PARAMS
      eepromCurrentValues.preinfusionFlowState          = lcdValues.preinfusionFlowState;
      eepromCurrentValues.preinfusionFlowVol            = lcdValues.preinfusionFlowVol;
      eepromCurrentValues.preinfusionFlowTime           = lcdValues.preinfusionFlowTime;
      eepromCurrentValues.preinfusionFlowSoakTime       = lcdValues.preinfusionFlowSoakTime;
      eepromCurrentValues.preinfusionFlowPressureTarget = lcdValues.preinfusionFlowPressureTarget;
      eepromCurrentValues.flowProfileState              = lcdValues.flowProfileState;
      eepromCurrentValues.flowProfileStart              = lcdValues.flowProfileStart;
      eepromCurrentValues.flowProfileEnd                = lcdValues.flowProfileEnd;
      eepromCurrentValues.flowProfilePressureTarget     = lcdValues.flowProfilePressureTarget;
      eepromCurrentValues.flowProfileCurveSpeed         = lcdValues.flowProfileCurveSpeed;
      break;
    case SCREEN_MODES::SCREEN_settings_boiler:
      eepromCurrentValues.setpoint                      = lcdValues.setpoint;
      eepromCurrentValues.steamSetPoint                 = lcdValues.steamSetPoint;
      eepromCurrentValues.offsetTemp                    = lcdValues.offsetTemp;
      eepromCurrentValues.hpwr                          = lcdValues.hpwr;
      eepromCurrentValues.mainDivider                   = lcdValues.mainDivider;
      eepromCurrentValues.brewDivider                   = lcdValues.brewDivider;
      break;
    case SCREEN_MODES::SCREEN_settings_system:
      eepromCurrentValues.powerLineFrequency            = lcdValues.powerLineFrequency;
      eepromCurrentValues.lcdSleep                      = lcdValues.lcdSleep;
      eepromCurrentValues.scalesF1                      = lcdValues.scalesF1;
      eepromCurrentValues.scalesF2                      = lcdValues.scalesF2;
      eepromCurrentValues.pumpFlowAtZero                = lcdValues.pumpFlowAtZero;
      break;
    case SCREEN_MODES::SCREEN_shot_settings:
      eepromCurrentValues.stopOnWeightState             = lcdValues.stopOnWeightState;
      eepromCurrentValues.shotDose                      = lcdValues.shotDose;
      eepromCurrentValues.shotPreset                    = lcdValues.shotPreset;
      eepromCurrentValues.shotStopOnCustomWeight        = lcdValues.shotStopOnCustomWeight;
      break;
    default:
      break;
  }

  rc = eepromWrite(eepromCurrentValues);
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

void lcdPumpPhaseShitfTrigger(void) {
  pumpPhaseShift();
  lcdShowPopup("Phase switched!");
}

//#############################################################################################
//###############################____PRESSURE_CONTROL____######################################
//#############################################################################################
static void updateProfilerPhases(void) {
  float preInfusionFinishBar = 0.f;
  float shotTarget = -1.f;
  float stopOnPressureAbove = -1;
  float switchPhaseOnDrip = -1;
  float pressureBelowRestriction = -1;
  float waterPumpedInPhase = -1;

  if (runningCfg.stopOnWeightState) {
    shotTarget = (runningCfg.shotStopOnCustomWeight < 1.f)
      ? runningCfg.shotDose * runningCfg.shotPreset
      : runningCfg.shotStopOnCustomWeight;
  }


  //update global stop conditions (currently only stopOnWeight is configured in nextion)
  profile.globalStopConditions = GlobalStopConditions{ .weight=shotTarget };

  profile.clear();
  //Setup release pressure + fill@4ml/sec
  if (runningCfg.basketPrefill) {
    addFillBasketPhase(4.5f);
  }

  // Setup pre-infusion if needed
  if (runningCfg.preinfusionState) {
    if (runningCfg.preinfusionFlowState) { // flow based PI enabled
      // For now handling phase switching on restrictions here but as this grow will have to deal with it otherwise.
      stopOnPressureAbove = runningCfg.switchPhaseOnThreshold ? runningCfg.preinfusionFlowPressureTarget : -1;
      switchPhaseOnDrip = runningCfg.switchOnWeightAbove > 0.f ? runningCfg.switchOnWeightAbove : -1;
      pressureBelowRestriction = runningCfg.switchPhaseOnPressureBelow > 0.f ? runningCfg.switchPhaseOnPressureBelow : -1;
      waterPumpedInPhase = runningCfg.switchOnWaterPumped > 0.f ? runningCfg.switchOnWaterPumped : -1;


      addFlowPhase(Transition{runningCfg.preinfusionFlowVol}, runningCfg.preinfusionFlowPressureTarget, runningCfg.preinfusionFlowTime * 1000, stopOnPressureAbove, -1, switchPhaseOnDrip, waterPumpedInPhase);
      addFlowPhase(Transition{0.f}, 0, runningCfg.preinfusionFlowSoakTime * 1000, -1, pressureBelowRestriction, switchPhaseOnDrip, -1);
      preInfusionFinishBar = fmaxf(0.f, runningCfg.preinfusionFlowPressureTarget);
    } else { // pressure based PI enabled
      // For now handling phase switching on restrictions here but as this grow will have to deal with it otherwise.
      stopOnPressureAbove = runningCfg.switchPhaseOnThreshold ? runningCfg.preinfusionBar : -1;
      switchPhaseOnDrip = runningCfg.switchOnWeightAbove > 0.f ? runningCfg.switchOnWeightAbove : -1;
      pressureBelowRestriction = runningCfg.switchPhaseOnPressureBelow > 0 ? runningCfg.switchPhaseOnPressureBelow : -1;
      waterPumpedInPhase = runningCfg.switchOnWaterPumped > 0.f ? runningCfg.switchOnWaterPumped : -1;

      addPressurePhase(Transition{(float) runningCfg.preinfusionBar}, 4.5f, runningCfg.preinfusionSec * 1000, stopOnPressureAbove, -1, switchPhaseOnDrip, waterPumpedInPhase);
      addPressurePhase(Transition{0.f}, -1, runningCfg.preinfusionSoak * 1000, -1, pressureBelowRestriction, switchPhaseOnDrip, -1);
      preInfusionFinishBar = runningCfg.preinfusionBar;
    }
  }
  preInfusionFinishedPhaseIdx = profile.phaseCount();

  // Setup shot profiling
  if (runningCfg.pressureProfilingState) {
    if (runningCfg.flowProfileState) { // flow based profiling enabled
      addFlowPhase(Transition{runningCfg.flowProfileStart, runningCfg.flowProfileEnd, TransitionCurve::LINEAR, runningCfg.flowProfileCurveSpeed * 1000}, runningCfg.flowProfilePressureTarget, -1, -1, -1, -1, -1);
    } else { // pressure based profiling enabled
      float ppStart = runningCfg.pressureProfilingStart;
      float ppEnd = runningCfg.pressureProfilingFinish;
      uint16_t rampAndHold = runningCfg.preinfusionRamp + runningCfg.pressureProfilingHold;
      addPressurePhase(Transition{preInfusionFinishBar, ppStart, TransitionCurve::EASE_OUT, runningCfg.preinfusionRamp * 1000}, -1, rampAndHold * 1000, -1, -1, -1, -1);
      addPressurePhase(Transition{ppStart, ppEnd, TransitionCurve::EASE_IN_OUT, runningCfg.pressureProfilingLength * 1000}, -1, -1, -1, -1, -1, -1);
    }
  } else { // Shot profiling disabled. Default to 9 bars
    addPressurePhase(Transition{preInfusionFinishBar, 9, TransitionCurve::EASE_OUT, runningCfg.preinfusionRamp * 1000}, -1, -1, -1, -1, -1, -1);
  }
}

void addFillBasketPhase(float flowRate) {
  addFlowPhase(Transition(flowRate), -1, -1, 0.1f, -1, -1, -1);
}

void addPressurePhase(Transition pressure, float flowRestriction, int timeMs, float pressureAbove, float pressureBelow, float shotWeight, float waterPumpedInPhase) {
  addPhase(PHASE_TYPE::PHASE_TYPE_PRESSURE, pressure, flowRestriction, timeMs, pressureAbove, pressureBelow, shotWeight, waterPumpedInPhase);
}

void addFlowPhase(Transition flow, float pressureRestriction, int timeMs, float pressureAbove, float pressureBelow, float shotWeight, float waterPumpedInPhase) {
  addPhase(PHASE_TYPE::PHASE_TYPE_FLOW, flow, pressureRestriction, timeMs, pressureAbove, pressureBelow, shotWeight, waterPumpedInPhase);
}

void addPhase(PHASE_TYPE type, Transition target, float restriction, int timeMs, float pressureAbove, float pressureBelow, float shotWeight, float waterPumpedInPhase) {
  profile.addPhase(Phase {
    .type           = type,
    .target         = target,
    .restriction    = restriction,
    .stopConditions = PhaseStopConditions{ .time=timeMs, .pressureAbove=pressureAbove, .pressureBelow=pressureBelow, .weight=shotWeight, .waterPumpedInPhase=waterPumpedInPhase }
  });
}

void onProfileReceived(Profile& newProfile) {
}

static void profiling(void) {
  if (brewActive) { //runs this only when brew button activated and pressure profile selected
    uint32_t timeInShot = millis() - brewingTimer;
    phaseProfiler.updatePhase(timeInShot, currentState, runningCfg);
    CurrentPhase& currentPhase = phaseProfiler.getCurrentPhase();
    preinfusionFinished = currentPhase.getIndex() >= preInfusionFinishedPhaseIdx;
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
  justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
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
  justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
}

//#############################################################################################
//###################################____BREW DETECT____#######################################
//#############################################################################################

static void brewDetect(void) {
  // Do not allow brew detection while system hasn't finished it's startup procedures.
  if (!systemState.startupInitFinished || !systemState.pumpCalibrationFinished) {
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
  preinfusionFinished      = false;
  brewingTimer             = millis();
  flowTimer                = millis() + REFRESH_FLOW_EVERY;
  systemHealthTimer        = millis() + HEALTHCHECK_EVERY;

  currentState.pumpClicks = getAndResetClickCounter();
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
      currentState.temperature  = thermocouple.readCelsius() - runningCfg.offsetTemp;  // Making sure we're getting a value
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
          brewDetect();
          lcdRefresh();
          lcdListen();
          sensorsRead();
          justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
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

static void calibratePump(void) {
  if (systemState.pumpCalibrationFinished) {
    return;
  }
  bool recalibrating = false;
  // Calibrate pump in both phases
  CALIBRATE_PHASES:
  lcdShowPopup(!recalibrating ? "Calibrating pump!" : "Re-calibrating!") ;
  for (int phase = 0; phase < 2; phase++) {
    watchdogReload();
    openValve();
    delay(1500);
    sensorsReadPressure();


    unsigned long loopTimeout = millis() + 2500L;
    // Wait for pressure to reach desired level.
    while (currentState.smoothedPressure < calibrationPressure) {
      watchdogReload();
      #if defined(SINGLE_BOARD) || defined(INDEPENDENT_DIMMER)
      closeValve();
      #endif
      setPumpToRawValue(50);
      if (currentState.smoothedPressure < 0.05f) {
        getAndResetClickCounter();
      }
      sensorsReadPressure();
      lcdRefresh();
      // Exit loop if timeout is reached.
      if (millis() > loopTimeout) {
        break;
      }
    }

    systemState.pumpClicks[phase] = getAndResetClickCounter();
    setPumpToRawValue(0);
    sensorsReadPressure();
    lcdSetPressure(currentState.smoothedPressure);
    lcdRefresh();
    openValve();

    // Switch pump phase for next calibration.
    if (phase < 1) pumpPhaseShift();
  }

  // Determine which phase has fewer clicks.
  long phaseDiffSanityCheck = systemState.pumpClicks[1] - systemState.pumpClicks[0];
  if ( systemState.pumpCalibrationRetries < 4 ) {
      if ((phaseDiffSanityCheck >= -2 && phaseDiffSanityCheck <= 2) || systemState.pumpClicks[0] <= 2 || systemState.pumpClicks[1] <= 2) {
      recalibrating = true;
      systemState.pumpCalibrationRetries++;
      goto CALIBRATE_PHASES;
    }
  }

  if (systemState.pumpClicks[1] < systemState.pumpClicks[0]) {
    pumpPhaseShift();
    lcdShowPopup("Pump phase [2]");
  }
  else if (systemState.pumpCalibrationRetries >= 4) {
    lcdShowPopup("Calibration failed!");
    systemState.startupInitFinished = true;
  }
  else lcdShowPopup("Pump phase [1]");

  // Set this var to true so phase is never repeated.
  systemState.pumpCalibrationFinished = true;
}
