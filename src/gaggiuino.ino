#if defined(DEBUG_ENABLED)
  #include "dbg.h"
#endif

#include "gaggiuino.h"

SimpleKalmanFilter smoothPressure(2, 2, 1.00);
SimpleKalmanFilter smoothPumpFlow(2, 2, 1.00);
SimpleKalmanFilter smoothScalesFlow(2, 2, 1.00);

//default phases. Updated in updatePressureProfilePhases.
Phase phaseArray[6];
Phases phases {6,  phaseArray};

SensorState currentState;

OPERATION_MODES selectedOperationalMode;

eepromValues_t runningCfg;

void setup(void) {
  LOG_INIT();
  LOG_INFO("Gaggiuino (fw: %s) booting", AUTO_VERSION);

  lcdInit();
  LOG_INFO("LCD Init");

  // Various pins operation mode handling
  pinInit();
  LOG_INFO("Pin init");

#if defined(DEBUG_ENABLED)
  // Debug init if enabled
  dbgInit();
  LOG_INFO("DBG init");
#endif

  setBoilerOff();  // relayPin LOW
  LOG_INFO("Boiler turned off");

  //Pump
  setPumpOff();
  LOG_INFO("Pump turned off");

  // Valve
  closeValve();
  LOG_INFO("Valve closed");

  // Initialising the vsaved values or writing defaults if first start
  eepromInit();
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  LOG_INFO("EEPROM Init");

  thermocoupleInit();
  LOG_INFO("Thermocouple Init");

  lcdUploadCfg(eepromCurrentValues);
  LOG_INFO("LCD cfg uploaded");

  pressureSensorInit();
  LOG_INFO("Pressure sensor init");

  // Scales handling
  scalesInit(eepromCurrentValues.scalesF1, eepromCurrentValues.scalesF2);
  LOG_INFO("Scales init");

  // Pump init
  pumpInit(eepromCurrentValues.powerLineFrequency, eepromCurrentValues.pumpFlowAtZero);
  LOG_INFO("Pump init");

  pageValuesRefresh(true);
  LOG_INFO("Setup sequence finished");
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
}

//##############################################################################################################################
//#############################################___________SENSORS_READ________##################################################
//##############################################################################################################################


static void sensorsRead(void) {
  sensorsReadTemperature();
  sensorsReadWeight();
  sensorsReadPressure();
  calculateWeightAndFlow();
  // monitorDripTrayState();
  fillBoiler(2.f);
}

static void sensorsReadTemperature(void) {
  if (millis() > thermoTimer) {
    currentState.temperature = thermocouple.readCelsius();
    /*
    This *while* is here to prevent situations where the system failed to get a temp reading and temp reads as 0 or -7(cause of the offset)
    If we would use a non blocking function then the system would keep the SSR in HIGH mode which would most definitely cause boiler overheating
    */
    while (currentState.temperature <= 0.0f || currentState.temperature  == NAN || currentState.temperature  >= 170.0f) {
      /* In the event of the temp failing to read while the SSR is HIGH
      we force set it to LOW while trying to get a temp reading - IMPORTANT safety feature */
      setBoilerOff();
      if (millis() > thermoTimer) {
        LOG_ERROR("Cannot read temp from thermocouple (last read: %.1lf)!", currentState.temperature);
        lcdShowPopup("TEMP READ ERROR"); // writing a LCD message
        currentState.temperature  = thermocouple.readCelsius();  // Making sure we're getting a value
        thermoTimer = millis() + GET_KTYPE_READ_EVERY;
      }
    }
    thermoTimer = millis() + GET_KTYPE_READ_EVERY;
  }
}

static void sensorsReadWeight(void) {
  if (scalesIsPresent() && millis() > scalesTimer) {
    if(!tareDone) {
      scalesTare(); //Tare at the start of any weighing cycle
      if (!nonBrewModeActive && (scalesGetWeight() < -0.1f || scalesGetWeight() > 0.1f)) tareDone = false;
      else tareDone = true;
    }
    currentState.weight = scalesGetWeight();
    scalesTimer = millis() + GET_SCALES_READ_EVERY;
  }
}

static void sensorsReadPressure(void) {
  if (millis() > pressureTimer) {
    currentState.pressure = getPressure();
    currentState.isPressureRising = isPressureRaising();
    currentState.isPressureFalling = isPressureFalling();
    currentState.isPressureFallingFast = isPressureFallingFast();
    smoothedPressure = smoothPressure.updateEstimate(currentState.pressure);
    pressureTimer = millis() + GET_PRESSURE_READ_EVERY;
  }
}

static void calculateWeightAndFlow(void) {
  if (brewActive) {
    if (scalesIsPresent()) {
      shotWeight = currentState.weight;
    }

    long elapsedTime = millis() - flowTimer;
    if (elapsedTime > REFRESH_FLOW_EVERY) {
      flowTimer = millis();
      // if(elapsedTime > 200) return;

      currentState.isOutputFlow = checkForOutputFlow(elapsedTime);

      if (scalesIsPresent()) {
        currentState.weightFlow = fmaxf(0.f, (shotWeight - previousWeight) * 1000.f / (float)elapsedTime);
        currentState.weightFlow = smoothScalesFlow.updateEstimate(currentState.weightFlow);
        previousWeight = shotWeight;
      } else if (currentState.isOutputFlow) {
        previousWeight = shotWeight; // temporary measure to avoid those 30 -45 grams sudden jumps
        shotWeight += smoothedPumpFlow * (float)elapsedTime / 1000.f;
        if (shotWeight > previousWeight + 5.f) shotWeight = 0.f; // temporary measure to avoid those 30 -45 grams sudden jumps
      }
      currentState.liquidPumped += smoothedPumpFlow * (float)elapsedTime / 1000.f;
    }
  }
}

bool checkForOutputFlow(long elapsedTime) {
  long pumpClicks = getAndResetClickCounter();
  float cps = 1000.f * (float)pumpClicks / (float)elapsedTime;

  float previousPumpFlow = currentState.pumpFlow;
  currentState.pumpFlow = getPumpFlow(cps, currentState.pressure);
  smoothedPumpFlow = smoothPumpFlow.updateEstimate(currentState.pumpFlow);
  currentState.isPumpFlowRisingFast = currentState.pumpFlow > previousPumpFlow + 0.45f;

  float lastResistance = currentState.puckResistance;
  currentState.puckResistance = smoothedPressure * 1000.f / smoothedPumpFlow; // Resistance in mBar * s / g
  float resistanceDelta = currentState.puckResistance - lastResistance;

  // If at least 60ml have been pumped, there has to be output (unless the water is going to the void)
  if (currentState.liquidPumped > 60.f) return true;
  else if (currentState.liquidPumped < 60.f
    && currentState.isPressureRising
    && preinfusionFinished
  ) currentState.isHeadSpaceFilled = true;

  if (!preinfusionFinished) {
    // If it's still in the preinfusion phase but didn't reach pressure nor pumped 45ml
    // it must have been a short ass preinfusion and it's blooming for some time
    if (currentState.liquidPumped < 45.f
      && ((
          smoothedPressure < (runningCfg.flowProfileState)
            ? runningCfg.preinfusionFlowPressureTarget - 0.9f
            : runningCfg.preinfusionBar - 0.9f
          )
      || currentState.isPressureFalling
      || currentState.pumpFlow < 0.2f)) {
      currentState.isHeadSpaceFilled = false;
      return false;
    }

    // If a certain amount of water has been pumped but no resistance is built up, there has to be output flow
    if (currentState.liquidPumped > 45.f && currentState.puckResistance > lastResistance
      && resistanceDelta > 0.f && resistanceDelta < 400.f) {
        currentState.isHeadSpaceFilled = true;
        return true;
      }
  // Theoretically, if resistance is still rising (resistanceDelta > 0), headspace should not be filled yet, hence no output flow.
  // Noisy readings make it impossible to use flat out, but it should at least somewhat work
  // Although a good threshold is very much experimental and not determined
  } else if (resistanceDelta >= 400.f || (currentState.isPressureRising && currentState.isPumpFlowRisingFast) || !currentState.isHeadSpaceFilled) {
    return false;
  } else return true;

  return false;
}

// Stops the pump if setting active and dose/weight conditions met
bool stopOnWeight() {
  if (!nonBrewModeActive && runningCfg.stopOnWeightState) {
    if (runningCfg.shotStopOnCustomWeight < 1.f)
      shotTarget = runningCfg.shotDose * runningCfg.shotPreset;
    else
      shotTarget = runningCfg.shotStopOnCustomWeight;
    if (shotWeight > (shotTarget - 0.5f) || brewStopWeight) {
      if (scalesIsPresent() && preinfusionFinished) brewStopWeight = shotWeight + currentState.weightFlow / 2.f;
      else brewStopWeight = shotWeight + smoothedPumpFlow / 2.f;
      return true;
    }
  }
  return false;
}
//##############################################################################################################################
//############################################______PAGE_CHANGE_VALUES_REFRESH_____#############################################
//##############################################################################################################################

static void pageValuesRefresh(bool forcedUpdate) {  // Refreshing our values on page changes

  if ( lcdCurrentPageId != lcdLastCurrentPageId || forcedUpdate == true ) {
    runningCfg = lcdDownloadCfg();


    homeScreenScalesEnabled = lcdGetHomeScreenScalesEnabled();
    selectedOperationalMode = (OPERATION_MODES) lcdGetSelectedOperationalMode(); // MODE_SELECT should always be LAST

    updatePressureProfilePhases();

    lcdLastCurrentPageId = lcdCurrentPageId;
  }
}

//#############################################################################################
//############################____OPERATIONAL_MODE_CONTROL____#################################
//#############################################################################################
static void modeSelect(void) {
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
      brewActive ? flushActivated() : flushDeactivated();
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
      lcdSetPressure(
        brewActive
          ? smoothedPressure * 10.f
          : currentState.pressure * 10.f
      );
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
      // lcdSetWeight(scalesDripTrayWeight());
    } else {
      if (scalesIsPresent()) {
        lcdSetWeight(
          (shotWeight > 0.f && !stopOnWeight())
            ? currentState.weight
            : brewStopWeight
        );
      } else if(shotWeight || brewStopWeight) {
        lcdSetWeight(
          (preinfusionFinished && !stopOnWeight())
          ? shotWeight
          : brewStopWeight
        );
      }
    }

    /*LCD flow output*/
    if (lcdCurrentPageId == 1 || lcdCurrentPageId == 2 || lcdCurrentPageId == 8 ) { // no point sending this continuously if on any other screens than brew related ones
      lcdSetFlow(
        currentState.weight > 0.4f // currentState.weight is always zero if scales are not present
          ? currentState.weightFlow * 10.f
          : smoothedPumpFlow * 10.f
      );
    }

#if defined(DEBUG_ENABLED)
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
      eepromCurrentValues.pressureProfilingStart    = lcdValues.pressureProfilingStart;
      eepromCurrentValues.pressureProfilingFinish   = lcdValues.pressureProfilingFinish;
      eepromCurrentValues.pressureProfilingHold     = lcdValues.pressureProfilingHold;
      eepromCurrentValues.pressureProfilingLength   = lcdValues.pressureProfilingLength;
      eepromCurrentValues.pressureProfilingState    = lcdValues.pressureProfilingState;
      eepromCurrentValues.preinfusionState          = lcdValues.preinfusionState;
      eepromCurrentValues.preinfusionSec            = lcdValues.preinfusionSec;
      eepromCurrentValues.preinfusionBar            = lcdValues.preinfusionBar;
      eepromCurrentValues.preinfusionSoak           = lcdValues.preinfusionSoak;
      eepromCurrentValues.preinfusionRamp           = lcdValues.preinfusionRamp;

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
      eepromCurrentValues.homeOnShotFinish  = lcdValues.homeOnShotFinish;
      eepromCurrentValues.graphBrew         = lcdValues.graphBrew;
      eepromCurrentValues.brewDeltaState    = lcdValues.brewDeltaState;
      eepromCurrentValues.warmupState       = lcdValues.warmupState;
      break;
    case 5:
      eepromCurrentValues.stopOnWeightState       = lcdValues.stopOnWeightState;
      eepromCurrentValues.shotDose                = lcdValues.shotDose;
      eepromCurrentValues.shotPreset              = lcdValues.shotPreset;
      eepromCurrentValues.shotStopOnCustomWeight  = lcdValues.shotStopOnCustomWeight;
      break;
    case 6:
      eepromCurrentValues.setpoint    = lcdValues.setpoint;
      eepromCurrentValues.offsetTemp  = lcdValues.offsetTemp;
      eepromCurrentValues.hpwr        = lcdValues.hpwr;
      eepromCurrentValues.mainDivider = lcdValues.mainDivider;
      eepromCurrentValues.brewDivider = lcdValues.brewDivider;
      break;
    case 7:
      eepromCurrentValues.powerLineFrequency = lcdValues.powerLineFrequency;
      eepromCurrentValues.lcdSleep           = lcdValues.lcdSleep;
      eepromCurrentValues.scalesF1           = lcdValues.scalesF1;
      eepromCurrentValues.scalesF2           = lcdValues.scalesF2;
      eepromCurrentValues.pumpFlowAtZero     = lcdValues.pumpFlowAtZero;
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

//#############################################################################################
//###############################____PRESSURE_CONTROL____######################################
//#############################################################################################
static void updatePressureProfilePhases(void) {
  int phaseCount = 0;
  float preInfusionFinishBar = 0.f;

  // Setup pre-infusion if needed
  if (runningCfg.preinfusionState) {
    if (runningCfg.preinfusionFlowState) { // flow based PI enabled
      setFlowPhase(phaseCount++, runningCfg.preinfusionFlowVol, runningCfg.preinfusionFlowVol, runningCfg.preinfusionFlowPressureTarget, runningCfg.preinfusionFlowTime * 1000);
      setFlowPhase(phaseCount++, 0, 0, 0, runningCfg.preinfusionFlowSoakTime * 1000);
    } else { // pressure based PI enabled
      setPressurePhase(phaseCount++, runningCfg.preinfusionBar, runningCfg.preinfusionBar, 4.f, runningCfg.preinfusionSec * 1000);
      setPressurePhase(phaseCount++, 0, 0, -1, runningCfg.preinfusionSoak * 1000);
      preInfusionFinishBar = runningCfg.preinfusionBar;
    }
  }
  preInfusionFinishedPhaseIdx = phaseCount;

  // Setup shot profiling
  if (runningCfg.pressureProfilingState) {
    if (runningCfg.flowProfileState) { // flow based profiling enabled
      setFlowPhase(phaseCount++, runningCfg.flowProfileStart, runningCfg.flowProfileEnd, runningCfg.flowProfilePressureTarget, runningCfg.flowProfileCurveSpeed * 1000);
    } else { // pressure based profiling enabled
      setPressurePhase(phaseCount++, preInfusionFinishBar, runningCfg.pressureProfilingStart, -1, runningCfg.preinfusionRamp * 1000);
      setPressurePhase(phaseCount++, runningCfg.pressureProfilingStart, runningCfg.pressureProfilingStart, -1, runningCfg.pressureProfilingHold * 1000);
      setPressurePhase(phaseCount++, runningCfg.pressureProfilingStart, runningCfg.pressureProfilingFinish, -1, runningCfg.pressureProfilingLength * 1000);
    }
  } else { // Shot profiling disabled. Default to 9 bars
    setPressurePhase(phaseCount++, preInfusionFinishBar, 9, -1, runningCfg.preinfusionRamp * 1000);
    setPressurePhase(phaseCount++, 9, 9, -1, 1000);
  }

  phases.count = phaseCount;
}

void setPressurePhase(int phaseIdx, float startBar, float endBar, float flowRestriction, int timeMs) {
  setPhase(phaseIdx, PHASE_TYPE_PRESSURE, startBar, endBar, flowRestriction, flowRestriction, timeMs);
}

void setFlowPhase(int phaseIdx, float startFlow, float endFlow, float pressureRestriction, int timeMs) {
  setPhase(phaseIdx, PHASE_TYPE_FLOW, startFlow, endFlow, pressureRestriction, pressureRestriction, timeMs);
}

void setPhase(int phaseIdx, PHASE_TYPE type, float startValue, float endValue, float startRestriction, float endRestriction, int timeMs) {
    phases.phases[phaseIdx].type = type;
    phases.phases[phaseIdx].startValue = startValue;
    phases.phases[phaseIdx].endValue = endValue;
    phases.phases[phaseIdx].startRestriction = startRestriction;
    phases.phases[phaseIdx].endRestriction = endRestriction;
    phases.phases[phaseIdx].durationMs = timeMs;
}

static void profiling(void) {

  if (brewActive) { //runs this only when brew button activated and pressure profile selected
    long timeInPP = millis() - brewingTimer;
    CurrentPhase currentPhase = phases.getCurrentPhase(timeInPP);
    preinfusionFinished = currentPhase.phaseIndex >= preInfusionFinishedPhaseIdx;

    if (phases.phases[currentPhase.phaseIndex].type == PHASE_TYPE_PRESSURE) {
      float newBarValue = phases.phases[currentPhase.phaseIndex].getTarget(currentPhase.timeInPhase);
      float flowRestriction =  phases.phases[currentPhase.phaseIndex].getRestriction(currentPhase.timeInPhase);
      setPumpPressure(newBarValue, flowRestriction, currentState);
    } else {
      float newFlowValue = phases.phases[currentPhase.phaseIndex].getTarget(currentPhase.timeInPhase);
      float pressureRestriction =  phases.phases[currentPhase.phaseIndex].getRestriction(currentPhase.timeInPhase);
      setPumpFlow(newFlowValue, pressureRestriction, currentState);
    }
  } else {
    if (startupInitFinished) setPumpToRawValue(0);
  }
  // Keep that water at temp
  justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
}

static void manualFlowControl(void) {
  float flow_reading = lcdGetManualPressurePower() / 10 ;
  setPumpFlow(flow_reading, 0.f, currentState);
  justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
}

//#############################################################################################
//###################################____BREW DETECT____#######################################
//#############################################################################################

static void brewDetect(void) {
  static bool paramsReset = true;

  if (brewState() && !stopOnWeight()) {
    if(!paramsReset) {
      brewParamsReset();
      paramsReset = true;
    }
    openValve();
    brewActive = true;
  } else {
    brewActive = false;
    if (startupInitFinished) {
      closeValve();
      setPumpOff();
    }
    if(!brewState() && paramsReset) {
      brewParamsReset();
      paramsReset = false;
    }
  }
}

static void brewParamsReset(void) {
  tareDone                        = false;
  shotWeight                      = 0.f;
  currentState.weight             = 0.f;
  currentState.liquidPumped       = 0.f;
  currentState.isHeadSpaceFilled  = false;
  brewStopWeight                  = 0.f;
  previousWeight                  = 0.f;
  brewingTimer                    = millis();
  preinfusionFinished             = false;
  lcdSendFakeTouch();
}


static void flushActivated(void) {
  setPumpFullOn();
  #ifdef SINGLE_BOARD
      openValve();
  #endif
}

static void flushDeactivated(void) {
  setPumpOff();
  #ifdef SINGLE_BOARD
      closeValve();
  #endif
}

static void fillBoiler(float targetBoilerFullPressure) {
  static float timePassed = millis();

  if (currentState.pressure < targetBoilerFullPressure && !startupInitFinished ) {
    openValve();
    setPumpToRawValue(80);
    if (millis() - timePassed > 5000.f) startupInitFinished = true;
  } else startupInitFinished = true;
}

static void monitorDripTrayState(void) {
  static bool dripTrayFull = false;
  float currentTotalTrayWeight = scalesDripTrayWeight();
  float actualTrayWeight  = currentTotalTrayWeight - EMPTY_TRAY_WEIGHT;

  if ( !brewActive && !steamState() ) {
    if (actualTrayWeight > TRAY_FULL_THRESHOLD) {
      if (millis() > trayTimer) {
        dripTrayFull = true;
        trayTimer = millis() + READ_TRAY_OFFSET_EVERY;
      }
    } else {
      trayTimer = millis() + READ_TRAY_OFFSET_EVERY;
      dripTrayFull = false;
    }
    if (dripTrayFull) lcdShowPopup("DRIP TRAY FULL");
  }
}
