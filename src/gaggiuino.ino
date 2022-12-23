#if defined(DEBUG_ENABLED)
  #include "dbg.h"
#endif

#include "gaggiuino.h"

SimpleKalmanFilter smoothPressure(2.f, 2.f, 0.5f);
SimpleKalmanFilter smoothPumpFlow(2.f, 2.f, 0.5f);
SimpleKalmanFilter smoothScalesFlow(2.f, 2.f, 0.5f);

//default phases. Updated in updateProfilerPhases.
Phase phaseArray[8];
Phases phases {8,  phaseArray};
PhaseProfiler phaseProfiler{phases};

PredictiveWeight predictiveWeight;

SensorState currentState;

OPERATION_MODES selectedOperationalMode;

eepromValues_t runningCfg;

void setup(void) {
  LOG_INIT();
  LOG_INFO("Gaggiuino (fw: %s) booting", AUTO_VERSION);

  // Various pins operation mode handling
  pinInit();
  LOG_INFO("Pin init");

  setBoilerOff();  // relayPin LOW
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

  // Initialising the vsaved values or writing defaults if first start
  eepromInit();
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  LOG_INFO("EEPROM Init");

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

  iwdcInit();
}

//##############################################################################################################################
//############################################________________MAIN______________################################################
//##############################################################################################################################


//Main loop where all the logic is continuously run
void loop(void) {
  pageValuesRefresh(false);
  lcdListen();
  sensorsRead();
  brewDetect();
  modeSelect();
  lcdRefresh();
  systemHealthCheck(0.8f);
}

//##############################################################################################################################
//#############################################___________SENSORS_READ________##################################################
//##############################################################################################################################


static void sensorsRead(void) {
  sensorsReadTemperature();
  sensorsReadWeight();
  sensorsReadPressure();
  calculateWeightAndFlow();
  fillBoiler(BOILER_FILL_PRESSURE);
}

static void sensorsReadTemperature(void) {
  if (millis() > thermoTimer) {
    currentState.temperature = thermocouple.readCelsius();
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
    currentState.isPressureRising = isPressureRaising();
    currentState.isPressureRisingFast = currentState.smoothedPressure >= previousSmoothedPressure + 0.06f;
    currentState.isPressureFalling = isPressureFalling();
    currentState.isPressureFallingFast = isPressureFallingFast();
    currentState.smoothedPressure = smoothPressure.updateEstimate(currentState.pressure);
    pressureTimer = millis() + GET_PRESSURE_READ_EVERY;
  }
}

static long sensorsReadFlow(float elapsedTime) {
    long pumpClicks = getAndResetClickCounter();
    float cps = 1000.f * (float)pumpClicks / elapsedTime;
    currentState.pumpFlow = getPumpFlow(cps, currentState.smoothedPressure);

    previousSmoothedPumpFlow = currentState.smoothedPumpFlow;
    currentState.smoothedPumpFlow = smoothPumpFlow.updateEstimate(currentState.pumpFlow);
    return pumpClicks;
}

static void calculateWeightAndFlow(void) {
  long elapsedTime = millis() - flowTimer;

  if (brewActive) {
    if (scalesIsPresent()) {
      currentState.shotWeight = currentState.weight;
    }

    if (elapsedTime > REFRESH_FLOW_EVERY) {
      flowTimer = millis();
      long pumpClicks = sensorsReadFlow(elapsedTime);
      currentState.isPumpFlowRisingFast = currentState.smoothedPumpFlow > previousSmoothedPumpFlow + 0.15f;
      currentState.isPumpFlowFallingFast = currentState.smoothedPumpFlow < previousSmoothedPumpFlow - 0.15f;

      bool previousIsOutputFlow = predictiveWeight.isOutputFlow();

      CurrentPhase& phase = phaseProfiler.getCurrentPhase();
      predictiveWeight.update(currentState, phase, runningCfg);

      if (scalesIsPresent()) {
        currentState.weightFlow = fmaxf(0.f, (currentState.shotWeight - previousWeight) * 1000.f / (float)elapsedTime);
        currentState.weightFlow = smoothScalesFlow.updateEstimate(currentState.weightFlow);
        previousWeight = currentState.shotWeight;
      } else if (predictiveWeight.isOutputFlow()) {
        float flowPerClick = getPumpFlowPerClick(currentState.smoothedPressure);
        // if the output flow just started, consider only 50% of the clicks (probabilistically).
        long consideredClicks = previousIsOutputFlow ? pumpClicks : pumpClicks * 0.5f;
        currentState.shotWeight += consideredClicks * flowPerClick;
      }
      currentState.waterPumped += currentState.smoothedPumpFlow * (float)elapsedTime / 1000.f;
    }
  } else {
    if (elapsedTime > REFRESH_FLOW_EVERY) {
      flowTimer = millis();
      sensorsReadFlow(elapsedTime);
    }
  }
}

//##############################################################################################################################
//############################################______PAGE_CHANGE_VALUES_REFRESH_____#############################################
//##############################################################################################################################

static void pageValuesRefresh(bool forcedUpdate) {  // Refreshing our values on page changes

  if ( lcdCurrentPageId != lcdLastCurrentPageId || forcedUpdate == true ) {
    runningCfg = lcdDownloadCfg();


    homeScreenScalesEnabled = lcdGetHomeScreenScalesEnabled();
    selectedOperationalMode = (OPERATION_MODES) lcdGetSelectedOperationalMode(); // MODE_SELECT should always be LAST

    updateProfilerPhases();

    lcdLastCurrentPageId = lcdCurrentPageId;
  }
}

//#############################################################################################
//############################____OPERATIONAL_MODE_CONTROL____#################################
//#############################################################################################
static void modeSelect(void) {
  if (!startupInitFinished) return;

  switch (selectedOperationalMode) {
    //REPLACE ALL THE BELOW WITH OPMODE_auto_profiling
    case OPMODE_straight9Bar:
    case OPMODE_justPreinfusion:
    case OPMODE_justPressureProfile:
    case OPMODE_preinfusionAndPressureProfile:
    case OPMODE_justFlowBasedProfiling:
    case OPMODE_justFlowBasedPreinfusion:
    case OPMODE_everythingFlowProfiled:
    case OPMODE_pressureBasedPreinfusionAndFlowProfile:
      nonBrewModeActive = false;
      if (!steamState()) profiling();
      else steamCtrl(runningCfg, currentState, brewActive);
      break;
    case OPMODE_manual:
      nonBrewModeActive = false;
      manualFlowControl();
      break;
    case OPMODE_flush:
      nonBrewModeActive = true;
      backFlush(currentState);
      justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
      break;
    case OPMODE_steam:
      nonBrewModeActive = true;
      if (!steamState()) {
        brewActive ? flushActivated() : flushDeactivated();
        justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
      } else {
        steamCtrl(runningCfg, currentState, brewActive);
      }
      break;
    case OPMODE_descale:
      nonBrewModeActive = true;
      deScale(runningCfg, currentState);
      break;
    case OPMODE_empty:
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
    lcdSetTemperature(currentState.temperature - runningCfg.offsetTemp);

    /*LCD weight output*/
    if (lcdCurrentPageId == 0 && homeScreenScalesEnabled) {
      lcdSetWeight(currentState.weight);
    } else {
      if (scalesIsPresent()) {
        lcdSetWeight(currentState.weight);
      } else if (currentState.shotWeight) {
        lcdSetWeight(currentState.shotWeight);
      }
    }

    /*LCD flow output*/
    if (lcdCurrentPageId == 1 || lcdCurrentPageId == 2 || lcdCurrentPageId == 8 ) { // no point sending this continuously if on any other screens than brew related ones
      lcdSetFlow(
        currentState.weight > 0.4f // currentState.weight is always zero if scales are not present
          ? currentState.weightFlow * 10.f
          : currentState.smoothedPumpFlow * 10.f
      );
    }

  #if defined DEBUG_ENABLED && defined stm32f411xx
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
void lcdTrigger1(void) {
  LOG_VERBOSE("Saving values to EEPROM");
  bool rc;
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  eepromValues_t lcdValues = lcdDownloadCfg();

  switch (lcdCurrentPageId){
    case 1:
      break;
    case 2:
      break;
    case 3:
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
    case 4:
      eepromCurrentValues.homeOnShotFinish              = lcdValues.homeOnShotFinish;
      eepromCurrentValues.basketPrefill                 = lcdValues.basketPrefill;
      eepromCurrentValues.brewDeltaState                = lcdValues.brewDeltaState;
      eepromCurrentValues.warmupState                   = lcdValues.warmupState;
      eepromCurrentValues.switchPhaseOnThreshold        = lcdValues.switchPhaseOnThreshold;
      break;
    case 5:
      eepromCurrentValues.stopOnWeightState             = lcdValues.stopOnWeightState;
      eepromCurrentValues.shotDose                      = lcdValues.shotDose;
      eepromCurrentValues.shotPreset                    = lcdValues.shotPreset;
      eepromCurrentValues.shotStopOnCustomWeight        = lcdValues.shotStopOnCustomWeight;
      break;
    case 6:
      eepromCurrentValues.setpoint                      = lcdValues.setpoint;
      eepromCurrentValues.steamSetPoint                 = lcdValues.steamSetPoint;
      eepromCurrentValues.offsetTemp                    = lcdValues.offsetTemp;
      eepromCurrentValues.hpwr                          = lcdValues.hpwr;
      eepromCurrentValues.mainDivider                   = lcdValues.mainDivider;
      eepromCurrentValues.brewDivider                   = lcdValues.brewDivider;
      break;
    case 7:
      eepromCurrentValues.powerLineFrequency            = lcdValues.powerLineFrequency;
      eepromCurrentValues.lcdSleep                      = lcdValues.lcdSleep;
      eepromCurrentValues.scalesF1                      = lcdValues.scalesF1;
      eepromCurrentValues.scalesF2                      = lcdValues.scalesF2;
      eepromCurrentValues.pumpFlowAtZero                = lcdValues.pumpFlowAtZero;
      break;
    default:
      break;
  }

  rc = eepromWrite(eepromCurrentValues);
  if (rc == true) {
    lcdShowPopup("UPDATE SUCCESSFUL!");
  } else {
    lcdShowPopup("ERROR!");
  }
}

void lcdTrigger2(void) {
  LOG_VERBOSE("Tare scales");
  if (scalesIsPresent()) scalesTare();
}

void lcdTrigger3(void) {
  LOG_VERBOSE("Scales enabled or disabled");
  homeScreenScalesEnabled = lcdGetHomeScreenScalesEnabled();
}

void lcdTrigger4(void) {
  LOG_VERBOSE("Predictive scales tare action completed!");
  if (!scalesIsPresent()) {
    if (currentState.shotWeight > 0.f) {
      currentState.shotWeight = 0.f;
      predictiveWeight.setIsForceStarted(true);
    } else predictiveWeight.setIsForceStarted(true);
  } else scalesTare();
}

//#############################################################################################
//###############################____PRESSURE_CONTROL____######################################
//#############################################################################################
static void updateProfilerPhases(void) {
  int phaseCount             = 0;
  float preInfusionFinishBar = 0.f;
  float shotTarget = -1.f;

  if (runningCfg.stopOnWeightState) {
    shotTarget = (runningCfg.shotStopOnCustomWeight < 1.f)
      ? runningCfg.shotDose * runningCfg.shotPreset
      : runningCfg.shotStopOnCustomWeight;
  }

  phaseProfiler.updateGlobalStopConditions(shotTarget);

  //Setup release pressure + fill@4ml/sec
  if (runningCfg.basketPrefill) {
    setFillBasketPhase(phaseCount++, 4.5f);
  }

  // Setup pre-infusion if needed
  if (runningCfg.preinfusionState) {
    if (runningCfg.preinfusionFlowState) { // flow based PI enabled
      float stopOnPressureAbove = (runningCfg.switchPhaseOnThreshold) ? runningCfg.preinfusionFlowPressureTarget : -1;
      setFlowPhase(phaseCount++, Transition{runningCfg.preinfusionFlowVol}, runningCfg.preinfusionFlowPressureTarget, runningCfg.preinfusionFlowTime * 1000, stopOnPressureAbove);
      setFlowPhase(phaseCount++, Transition{0.f}, 0, runningCfg.preinfusionFlowSoakTime * 1000, -1);
      preInfusionFinishBar = fmaxf(0.f, runningCfg.preinfusionFlowPressureTarget);
    } else { // pressure based PI enabled
      float stopOnPressureAbove = (runningCfg.switchPhaseOnThreshold) ? runningCfg.preinfusionBar : -1;
      setPressurePhase(phaseCount++, Transition{(float) runningCfg.preinfusionBar}, 4.5f, runningCfg.preinfusionSec * 1000, stopOnPressureAbove);
      setPressurePhase(phaseCount++, Transition{0.f}, -1, runningCfg.preinfusionSoak * 1000, -1);
      preInfusionFinishBar = runningCfg.preinfusionBar;
    }
  }
  preInfusionFinishedPhaseIdx = phaseCount;

  // Setup shot profiling
  if (runningCfg.pressureProfilingState) {
    if (runningCfg.flowProfileState) { // flow based profiling enabled
      setFlowPhase(phaseCount++, Transition{runningCfg.flowProfileStart, runningCfg.flowProfileEnd, LINEAR, runningCfg.flowProfileCurveSpeed * 1000}, runningCfg.flowProfilePressureTarget, -1, -1);
    } else { // pressure based profiling enabled
      float ppStart = runningCfg.pressureProfilingStart;
      float ppEnd = runningCfg.pressureProfilingFinish;
      uint16_t rampAndHold = runningCfg.preinfusionRamp + runningCfg.pressureProfilingHold;
      setPressurePhase(phaseCount++, Transition{preInfusionFinishBar, ppStart, EASE_OUT, runningCfg.preinfusionRamp * 1000}, -1, rampAndHold * 1000, -1);
      setPressurePhase(phaseCount++, Transition{ppStart, ppEnd, EASE_IN_OUT, runningCfg.pressureProfilingLength * 1000}, -1, -1, -1);
    }
  } else { // Shot profiling disabled. Default to 9 bars
    setPressurePhase(phaseCount++, Transition{preInfusionFinishBar, 9, EASE_OUT, runningCfg.preinfusionRamp * 1000}, -1, -1, -1);
  }

  phases.count = phaseCount;
}

void setFillBasketPhase(int phaseIdx, float flowRate) {
  setFlowPhase(phaseIdx, Transition(flowRate), -1, -1, 0.1f);
}

void setPressurePhase(int phaseIdx, Transition pressure, float flowRestriction, int timeMs, float pressureAbove) {
  setPhase(phaseIdx, PHASE_TYPE_PRESSURE, pressure, flowRestriction, timeMs, pressureAbove);
}

void setFlowPhase(int phaseIdx, Transition flow, float pressureRestriction, int timeMs, float pressureAbove) {
  setPhase(phaseIdx, PHASE_TYPE_FLOW, flow, pressureRestriction, timeMs, pressureAbove);
}

void setPhase(
  int phaseIdx,
  PHASE_TYPE type,
  Transition target,
  float restriction,
  int timeMs,
  float pressureAbove
) {
  phases.phases[phaseIdx].type              = type;
  phases.phases[phaseIdx].target            = target;
  phases.phases[phaseIdx].restriction       = restriction;
  phases.phases[phaseIdx].stopConditions    = PhaseStopConditions{
    timeMs,
    pressureAbove,
    -1, // pressureBelow
    -1, // flowAbove
    -1, // flowBelow
    -1, // weight
  };
}

static void profiling(void) {
  if (brewActive) { //runs this only when brew button activated and pressure profile selected
    long timeInShot = millis() - brewingTimer;
    phaseProfiler.updatePhase(timeInShot, currentState);
    CurrentPhase& currentPhase = phaseProfiler.getCurrentPhase();
    preinfusionFinished = currentPhase.getIndex() >= preInfusionFinishedPhaseIdx;

    if (phaseProfiler.isFinished()) {
      closeValve();
      setPumpOff();
      brewActive = false;
    } else if (currentPhase.getType() == PHASE_TYPE_PRESSURE) {
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
    float flow_reading = lcdGetManualFlowVol() / 10 ;
    setPumpFlow(flow_reading, 0.f, currentState);
    justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
  } else {
    closeValve();
    setPumpOff();
  }
}

//#############################################################################################
//###################################____BREW DETECT____#######################################
//#############################################################################################

static void brewDetect(void) {
  static bool paramsReset = true;

  if (brewState()) {
    if(!paramsReset) {
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
  tareDone                                    = false;
  currentState.shotWeight                     = 0.f;
  currentState.pumpFlow                       = 0.f;
  previousWeight                              = 0.f;
  currentState.weight                         = 0.f;
  currentState.waterPumped                    = 0.f;
  preinfusionFinished                         = false;
  brewingTimer                                = millis();
  flowTimer                                   = millis() + REFRESH_FLOW_EVERY;
  systemHealthTimer                           = millis() + HEALTHCHECK_EVERY;

  predictiveWeight.reset();
  phaseProfiler.reset();
}

static void fillBoiler(float targetBoilerFullPressure) {
  static long elapsedTimeSinceStart = millis();
#if defined LEGO_VALVE_RELAY || defined SINGLE_BOARD
  lcdSetUpTime((millis() > elapsedTimeSinceStart) ? (int)((millis() - elapsedTimeSinceStart) / 1000) : 0);
  if (!startupInitFinished && lcdCurrentPageId == 0 && millis() - elapsedTimeSinceStart >= 3000) {
    unsigned long timePassed = millis() - elapsedTimeSinceStart;

    if (currentState.smoothedPressure < targetBoilerFullPressure && timePassed <= BOILER_FILL_TIMEOUT) {
      lcdShowPopup("Filling boiler!");
      openValve();
      setPumpToRawValue(80);
    } else if (!startupInitFinished) {
      closeValve();
      setPumpToRawValue(0);
      startupInitFinished = true;
    }
  }
#else
  startupInitFinished = true;
#endif
}

static void systemHealthCheck(float pressureThreshold) {
  //Reloading the watchdog timer, if this function fails to run MCU is rebooted
  IWatchdog.reload();

  //Releasing the excess pressure after steaming or brewing if necessary
  #if defined LEGO_VALVE_RELAY || defined SINGLE_BOARD
  if (!brewState() && !steamState()) {
    if (millis() >= systemHealthTimer) {
      while (currentState.smoothedPressure >= pressureThreshold && currentState.temperature < STEAM_WAND_HOT_WATER_TEMP)
      {
        //Reloading the watchdog timer, if this function fails to run MCU is rebooted
        IWatchdog.reload();
        lcdShowPopup("Releasing pressure!");
        setPumpOff();
        setBoilerOff();
        openValve();
        sensorsRead();
      }
      closeValve();
      systemHealthTimer = millis() + HEALTHCHECK_EVERY;
    }
  }
  #endif

  /* This *while* is here to prevent situations where the system failed to get a temp reading and temp reads as 0 or -7(cause of the offset)
  If we would use a non blocking function then the system would keep the SSR in HIGH mode which would most definitely cause boiler overheating */
  while (currentState.temperature <= 0.0f || currentState.temperature  == NAN || currentState.temperature  >= 170.0f) {
    //Reloading the watchdog timer, if this function fails to run MCU is rebooted
    IWatchdog.reload();
    /* In the event of the temp failing to read while the SSR is HIGH
    we force set it to LOW while trying to get a temp reading - IMPORTANT safety feature */
    setPumpOff();
    setBoilerOff();
    if (millis() > thermoTimer) {
      LOG_ERROR("Cannot read temp from thermocouple (last read: %.1lf)!", static_cast<double>(currentState.temperature));
      lcdShowPopup("TEMP READ ERROR"); // writing a LCD message
      currentState.temperature  = thermocouple.readCelsius();  // Making sure we're getting a value
      thermoTimer = millis() + GET_KTYPE_READ_EVERY;
    }
  }
}

/*Checking whether system is booting after a hard reset initiated by the internal watchdog.*/
void iwdcInit(void) {
  // IWDC init
  if(IWatchdog.isReset()) {
    lcdShowPopup("WATCHDOG RESTARTED");
    IWatchdog.clearReset();
  }
  IWatchdog.begin(3000000);
  LOG_INFO("Internal watchdog Init");
}
