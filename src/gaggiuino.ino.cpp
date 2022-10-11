# 1 "C:\\Users\\victo\\AppData\\Local\\Temp\\tmpu4kmd2k0"
#include <Arduino.h>
# 1 "C:/GIT/gaggiuino/src/gaggiuino.ino"
#if defined(DEBUG_ENABLED)
  #include "dbg.h"
#endif

#include "gaggiuino.h"

SimpleKalmanFilter smoothPressure(1.f, 1.f, 1.f);
SimpleKalmanFilter smoothPumpFlow(2.f, 2.f, 1.5f);
SimpleKalmanFilter smoothScalesFlow(2.f, 2.f, 1.5f);


Phase phaseArray[6];
Phases phases {6, phaseArray};


SensorState currentState;

OPERATION_MODES selectedOperationalMode;

eepromValues_t runningCfg;

StageRestrict stageRestrict;
void setup(void);
void loop(void);
static void sensorsRead(void);
static void sensorsReadTemperature(void);
static void sensorsReadWeight(void);
static void sensorsReadPressure(void);
static void calculateWeightAndFlow(void);
bool checkForOutputFlow(long elapsedTime);
bool stopOnWeight();
static void pageValuesRefresh(bool forcedUpdate);
static void brewTimeConditionsRefresh(void);
static void modeSelect(void);
static void lcdRefresh(void);
void lcdTrigger1(void);
void lcdTrigger2(void);
void lcdTrigger3(void);
void lcdTrigger4(void);
static void updatePressureProfilePhases(void);
void setPressurePhase(int phaseIdx, STAGE_TYPE stage, float startBar, float endBar, float flowRestriction, int timeMs, float pressureTarget, float weightTarget);
void setFlowPhase(int phaseIdx, STAGE_TYPE stage, float startFlow, float endFlow, float pressureRestriction, int timeMs, float pressureTarget, float weightTarget);
void setPhase(int phaseIdx, PHASE_TYPE type, STAGE_TYPE stage, float startValue, float endValue, float startRestriction, float endRestriction, int timeMs, float pressureTarget, float weightTarget);
static void profiling(void);
static void manualFlowControl(void);
static void brewDetect(void);
static void brewParamsReset(void);
static void flushActivated(void);
static void flushDeactivated(void);
static void fillBoiler(float targetBoilerFullPressure);
static void systemHealthCheck(float pressureThreshold);
#line 24 "C:/GIT/gaggiuino/src/gaggiuino.ino"
void setup(void) {
  LOG_INIT();
  LOG_INFO("Gaggiuino (fw: %s) booting", AUTO_VERSION);


  pinInit();
  LOG_INFO("Pin init");

  setBoilerOff();
  LOG_INFO("Boiler turned off");


  setPumpOff();
  LOG_INFO("Pump turned off");


  closeValve();
  LOG_INFO("Valve closed");

  lcdInit();
  LOG_INFO("LCD Init");

#if defined(DEBUG_ENABLED)

  dbgInit();
  LOG_INFO("DBG init");
#endif


  eepromInit();
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  LOG_INFO("EEPROM Init");

  thermocoupleInit();
  LOG_INFO("Thermocouple Init");

  lcdUploadCfg(eepromCurrentValues);
  LOG_INFO("LCD cfg uploaded");

  pressureSensorInit();
  LOG_INFO("Pressure sensor init");


  scalesInit(eepromCurrentValues.scalesF1, eepromCurrentValues.scalesF2);
  LOG_INFO("Scales init");


  pumpInit(eepromCurrentValues.powerLineFrequency, eepromCurrentValues.pumpFlowAtZero);
  LOG_INFO("Pump init");

  pageValuesRefresh(true);
  LOG_INFO("Setup sequence finished");
}







void loop(void) {
  pageValuesRefresh(false);
  lcdListen();
  sensorsRead();
  brewDetect();
  modeSelect();
  lcdRefresh();
  systemHealthCheck(0.8f);
}






static void sensorsRead(void) {
  sensorsReadTemperature();
  sensorsReadWeight();
  sensorsReadPressure();
  calculateWeightAndFlow();
  brewTimeConditionsRefresh();
  fillBoiler(2.2f);
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
      scalesTare();
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

static void calculateWeightAndFlow(void) {
  if (brewActive) {
    if (scalesIsPresent()) {
      shotWeight = currentState.weight;
    }

    long elapsedTime = millis() - flowTimer;
    if (elapsedTime > REFRESH_FLOW_EVERY) {
      flowTimer = millis();


      currentState.isOutputFlow = checkForOutputFlow(elapsedTime);

      if (scalesIsPresent()) {
        currentState.weightFlow = fmaxf(0.f, (shotWeight - previousWeight) * 1000.f / (float)elapsedTime);
        currentState.weightFlow = smoothScalesFlow.updateEstimate(currentState.weightFlow);
        previousWeight = shotWeight;
      } else if (currentState.isOutputFlow) {
        previousWeight = shotWeight;
        shotWeight += currentState.smoothedPumpFlow * (float)elapsedTime / 1000.f;
        if (shotWeight > previousWeight + 5.f) shotWeight = 0.f;
      }
      currentState.liquidPumped += currentState.smoothedPumpFlow * (float)elapsedTime / 1000.f;
    }
  }
}

bool checkForOutputFlow(long elapsedTime) {
  long pumpClicks = getAndResetClickCounter();
  float cps = 1000.f * (float)pumpClicks / (float)elapsedTime;

  float previousPumpFlow = currentState.pumpFlow;
  previousSmoothedPumpFlow = currentState.smoothedPumpFlow;
  currentState.pumpFlow = getPumpFlow(cps, currentState.pressure);
  currentState.smoothedPumpFlow = smoothPumpFlow.updateEstimate(currentState.pumpFlow);
  currentState.isPumpFlowRisingFast = currentState.smoothedPumpFlow > previousSmoothedPumpFlow + 0.45f;


  if (currentState.isPredictiveWeightForceStarted) return true;

  float lastResistance = currentState.puckResistance;
  currentState.puckResistance = currentState.smoothedPressure * 1000.f / currentState.smoothedPumpFlow;
  float resistanceDelta = currentState.puckResistance - lastResistance;


  if (currentState.liquidPumped > 60.f || currentState.isHeadSpaceFilled) return true;
  else if (currentState.liquidPumped <= 60.f) {
    if (preinfusionFinished && (!currentState.isPumpFlowRisingFast || !currentState.isPressureRisingFast)) {
      (pumpClicks < 70 && currentState.puckResistance > 1500) ? currentState.isHeadSpaceFilled = true : currentState.isHeadSpaceFilled = false;
    }
    else if (!runningCfg.preinfusionState) {
      if (resistanceDelta < 200 && currentState.puckResistance >= 1500) return true;
      else return false;
    }
    else if (currentState.puckResistance > 1500) {
      currentState.isHeadSpaceFilled = true;
    } else currentState.isHeadSpaceFilled = false;
  }

  if (!preinfusionFinished) {


    if (currentState.liquidPumped < 45.f
      && ((
          currentState.smoothedPressure < (runningCfg.flowProfileState)
            ? runningCfg.preinfusionFlowPressureTarget - 0.6f
            : runningCfg.preinfusionBar - 0.6f
          )
      || currentState.isPressureFalling
      || currentState.smoothedPumpFlow < 0.2f)) {
      currentState.isHeadSpaceFilled = false;
      return false;
    }


    if (currentState.liquidPumped > 45.f && currentState.puckResistance <= 500.f) {
        currentState.isHeadSpaceFilled = true;
        return true;
      }



  } else if (resistanceDelta > 600.f || currentState.isPumpFlowRisingFast || !currentState.isHeadSpaceFilled) return false;
  else return true;

  return false;
}


bool stopOnWeight() {
  if (!nonBrewModeActive && runningCfg.stopOnWeightState) {
    if (runningCfg.shotStopOnCustomWeight < 1.f)
      shotTarget = runningCfg.shotDose * runningCfg.shotPreset;
    else
      shotTarget = runningCfg.shotStopOnCustomWeight;
    if (shotWeight > (shotTarget - 0.5f) || brewStopWeight) {
      if (scalesIsPresent() && preinfusionFinished) brewStopWeight = shotWeight + currentState.weightFlow / 2.f;
      else brewStopWeight = shotWeight + currentState.smoothedPumpFlow / 2.f;
      return true;
    }
  }
  return false;
}




static void pageValuesRefresh(bool forcedUpdate) {

  if ( lcdCurrentPageId != lcdLastCurrentPageId || forcedUpdate == true ) {
    runningCfg = lcdDownloadCfg();


    homeScreenScalesEnabled = lcdGetHomeScreenScalesEnabled();
    selectedOperationalMode = (OPERATION_MODES) lcdGetSelectedOperationalMode();

    updatePressureProfilePhases();

    lcdLastCurrentPageId = lcdCurrentPageId;
  }
}

static void brewTimeConditionsRefresh(void) {
  if (brewActive) {
    if (runningCfg.switchPhaseOnThreshold) updatePressureProfilePhases();
  }
}



static void modeSelect(void) {
  switch (selectedOperationalMode) {

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





static void lcdRefresh(void) {

  if (millis() > pageRefreshTimer) {

    #ifdef BEAUTIFY_GRAPH
      lcdSetPressure(
        brewActive
          ? currentState.smoothedPressure * 10.f
          : currentState.pressure * 10.f
      );
    #else
      lcdSetPressure(
        currentState.pressure > 0.f
          ? currentState.pressure * 10.f
          : 0.f
      );
    #endif


    lcdSetTemperature(currentState.temperature - runningCfg.offsetTemp);


    if (lcdCurrentPageId == 0 && homeScreenScalesEnabled) {
      lcdSetWeight(currentState.weight);

    } else {
      if (scalesIsPresent()) {
        lcdSetWeight(
          (shotWeight > 0.f && !stopOnWeight())
            ? currentState.weight
            : brewStopWeight
        );
      } else if (shotWeight || brewStopWeight) {
        lcdSetWeight(
          stopOnWeight()
          ? brewStopWeight
          : shotWeight
        );
      }
    }


    if (lcdCurrentPageId == 1 || lcdCurrentPageId == 2 || lcdCurrentPageId == 8 ) {
      lcdSetFlow(
        currentState.weight > 0.4f
          ? currentState.weightFlow * 10.f
          : currentState.smoothedPumpFlow * 10.f
      );
    }

#if defined(DEBUG_ENABLED)
    lcdShowDebug(readTempSensor(), getAdsError());
#endif


    if (brewActive) {
      lcdSetBrewTimer((millis() > brewingTimer) ? (int)((millis() - brewingTimer) / 1000) : 0);
      lcdBrewTimerStart();
      lcdWarmupStateStop();
    } else {
      lcdBrewTimerStop();
    }

    pageRefreshTimer = millis() + REFRESH_SCREEN_EVERY;
  }
}




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

      eepromCurrentValues.pressureProfilingStart = lcdValues.pressureProfilingStart;
      eepromCurrentValues.pressureProfilingFinish = lcdValues.pressureProfilingFinish;
      eepromCurrentValues.pressureProfilingHold = lcdValues.pressureProfilingHold;
      eepromCurrentValues.pressureProfilingLength = lcdValues.pressureProfilingLength;
      eepromCurrentValues.pressureProfilingState = lcdValues.pressureProfilingState;
      eepromCurrentValues.preinfusionState = lcdValues.preinfusionState;
      eepromCurrentValues.preinfusionSec = lcdValues.preinfusionSec;
      eepromCurrentValues.preinfusionBar = lcdValues.preinfusionBar;
      eepromCurrentValues.preinfusionSoak = lcdValues.preinfusionSoak;
      eepromCurrentValues.preinfusionRamp = lcdValues.preinfusionRamp;


      eepromCurrentValues.preinfusionFlowState = lcdValues.preinfusionFlowState;
      eepromCurrentValues.preinfusionFlowVol = lcdValues.preinfusionFlowVol;
      eepromCurrentValues.preinfusionFlowTime = lcdValues.preinfusionFlowTime;
      eepromCurrentValues.preinfusionFlowSoakTime = lcdValues.preinfusionFlowSoakTime;
      eepromCurrentValues.preinfusionFlowPressureTarget = lcdValues.preinfusionFlowPressureTarget;
      eepromCurrentValues.flowProfileState = lcdValues.flowProfileState;
      eepromCurrentValues.flowProfileStart = lcdValues.flowProfileStart;
      eepromCurrentValues.flowProfileEnd = lcdValues.flowProfileEnd;
      eepromCurrentValues.flowProfilePressureTarget = lcdValues.flowProfilePressureTarget;
      eepromCurrentValues.flowProfileCurveSpeed = lcdValues.flowProfileCurveSpeed;
      break;
    case 4:
      eepromCurrentValues.homeOnShotFinish = lcdValues.homeOnShotFinish;
      eepromCurrentValues.graphBrew = lcdValues.graphBrew;
      eepromCurrentValues.brewDeltaState = lcdValues.brewDeltaState;
      eepromCurrentValues.warmupState = lcdValues.warmupState;
      eepromCurrentValues.switchPhaseOnThreshold = lcdValues.switchPhaseOnThreshold;
      break;
    case 5:
      eepromCurrentValues.stopOnWeightState = lcdValues.stopOnWeightState;
      eepromCurrentValues.shotDose = lcdValues.shotDose;
      eepromCurrentValues.shotPreset = lcdValues.shotPreset;
      eepromCurrentValues.shotStopOnCustomWeight = lcdValues.shotStopOnCustomWeight;
      break;
    case 6:
      eepromCurrentValues.setpoint = lcdValues.setpoint;
      eepromCurrentValues.offsetTemp = lcdValues.offsetTemp;
      eepromCurrentValues.hpwr = lcdValues.hpwr;
      eepromCurrentValues.mainDivider = lcdValues.mainDivider;
      eepromCurrentValues.brewDivider = lcdValues.brewDivider;
      break;
    case 7:
      eepromCurrentValues.powerLineFrequency = lcdValues.powerLineFrequency;
      eepromCurrentValues.lcdSleep = lcdValues.lcdSleep;
      eepromCurrentValues.scalesF1 = lcdValues.scalesF1;
      eepromCurrentValues.scalesF2 = lcdValues.scalesF2;
      eepromCurrentValues.pumpFlowAtZero = lcdValues.pumpFlowAtZero;
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
    if (shotWeight > 0.f) {
      shotWeight = 0.f;
      currentState.isPredictiveWeightForceStarted = true;
    } else currentState.isPredictiveWeightForceStarted = true;
  } else scalesTare();
}




static void updatePressureProfilePhases(void) {
  int phaseCount = 0;
  float preInfusionFinishBar = 0.f;


  if (runningCfg.preinfusionState) {
    PhaseConditions phaseConditions = { millis() - brewingTimer};
    CurrentPhase currentPhase = phases.getCurrentPhase(phaseConditions);

    if (runningCfg.preinfusionFlowState) {
      setFlowPhase(phaseCount++, STAGE_PI_FILL, runningCfg.preinfusionFlowVol, runningCfg.preinfusionFlowVol, runningCfg.preinfusionFlowPressureTarget, stageRestrict.piStageTime * 1000, runningCfg.preinfusionFlowPressureTarget, -1);
      setFlowPhase(phaseCount++, STAGE_PI_SOAK,0 , 0, 0, runningCfg.preinfusionFlowSoakTime * 1000, -1, -1);
    } else {
      setPressurePhase(phaseCount++, STAGE_PI_FILL, runningCfg.preinfusionBar, runningCfg.preinfusionBar, 4.f, stageRestrict.piStageTime * 1000, runningCfg.preinfusionBar, -1);
      setPressurePhase(phaseCount++, STAGE_PI_SOAK, 0, 0, -1, runningCfg.preinfusionSoak * 1000, -1, -1);
      preInfusionFinishBar = runningCfg.preinfusionBar;
    }
  }
  preInfusionFinishedPhaseIdx = phaseCount;


  if (runningCfg.pressureProfilingState) {
    if (runningCfg.flowProfileState) {
      setFlowPhase(phaseCount++, STAGE_FP_MAIN, runningCfg.flowProfileStart, runningCfg.flowProfileEnd, runningCfg.flowProfilePressureTarget, runningCfg.flowProfileCurveSpeed * 1000, -1, shotTarget);
    } else {
      setPressurePhase(phaseCount++, STAGE_PI_RAMP, preInfusionFinishBar, runningCfg.pressureProfilingStart, -1, runningCfg.preinfusionRamp * 1000, -1, -1);
      setPressurePhase(phaseCount++, STAGE_PP_HOLD, runningCfg.pressureProfilingStart, runningCfg.pressureProfilingStart, -1, runningCfg.pressureProfilingHold * 1000, -1, shotTarget);
      setPressurePhase(phaseCount++, STAGE_PP_MAIN, runningCfg.pressureProfilingStart, runningCfg.pressureProfilingFinish, -1, runningCfg.pressureProfilingLength * 1000, -1, shotTarget);
    }
  } else {
    setPressurePhase(phaseCount++, STAGE_PI_RAMP, preInfusionFinishBar, 9, -1, runningCfg.preinfusionRamp * 1000, -1, shotTarget);
    setPressurePhase(phaseCount++, STAGE_PP_MAIN, 9, 9, -1, 1000, -1, shotTarget);
  }

  phases.count = phaseCount;
}

void setPressurePhase(int phaseIdx, STAGE_TYPE stage, float startBar, float endBar, float flowRestriction, int timeMs, float pressureTarget, float weightTarget) {
  setPhase(phaseIdx, PHASE_TYPE_PRESSURE, stage, startBar, endBar, flowRestriction, flowRestriction, timeMs, pressureTarget, weightTarget);
}

void setFlowPhase(int phaseIdx, STAGE_TYPE stage, float startFlow, float endFlow, float pressureRestriction, int timeMs, float pressureTarget, float weightTarget) {
  setPhase(phaseIdx, PHASE_TYPE_FLOW, stage, startFlow, endFlow, pressureRestriction, pressureRestriction, timeMs, pressureTarget, weightTarget);
}

void setPhase(int phaseIdx, PHASE_TYPE type, STAGE_TYPE stage, float startValue, float endValue, float startRestriction, float endRestriction, int timeMs, float pressureTarget, float weightTarget) {
    phases.phases[phaseIdx].type = type;
    phases.phases[phaseIdx].stage = stage;
    phases.phases[phaseIdx].startValue = startValue;
    phases.phases[phaseIdx].endValue = endValue;
    phases.phases[phaseIdx].startRestriction = startRestriction;
    phases.phases[phaseIdx].endRestriction = endRestriction;
    phases.phases[phaseIdx].durationMs = timeMs;
    phases.phases[phaseIdx].pressureTarget = pressureTarget;
    phases.phases[phaseIdx].shotTarget = weightTarget;
}

static void profiling(void) {
  static bool pressureThresholdReached = false;

  if (brewActive) {

    PhaseConditions phaseConditions = { millis() - brewingTimer };
    CurrentPhase currentPhase = phases.getCurrentPhase(phaseConditions);

    preinfusionFinished = currentPhase.phaseIndex >= preInfusionFinishedPhaseIdx;

    if (phases.phases[currentPhase.phaseIndex].type == PHASE_TYPE_PRESSURE) {
      if (runningCfg.switchPhaseOnThreshold) {
        stageRestrict.piStageTime = phases.phases[currentPhase.phaseIndex].getTimeRestriction(currentState, runningCfg);
      } else stageRestrict.piStageTime = runningCfg.preinfusionSec;

      float newBarValue = phases.phases[currentPhase.phaseIndex].getTarget(currentPhase.timeInPhase);
      float flowRestriction = phases.phases[currentPhase.phaseIndex].getRestriction(currentPhase.timeInPhase);
      setPumpPressure(newBarValue, flowRestriction, currentState);
    } else {
      if (runningCfg.switchPhaseOnThreshold) {
        stageRestrict.piStageTime = phases.phases[currentPhase.phaseIndex].getTimeRestriction(currentState, runningCfg);
      } else stageRestrict.piStageTime = runningCfg.preinfusionFlowTime;

      float newFlowValue = phases.phases[currentPhase.phaseIndex].getTarget(currentPhase.timeInPhase);
      float pressureRestriction = phases.phases[currentPhase.phaseIndex].getRestriction(currentPhase.timeInPhase);
      setPumpFlow(newFlowValue, pressureRestriction, currentState);
    }
  } else {
    if (startupInitFinished) setPumpToRawValue(0);
    pressureThresholdReached = false;
  }

  justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
}

static void manualFlowControl(void) {
  float flow_reading = lcdGetManualFlowVol() / 10 ;
  setPumpFlow(flow_reading, 0.f, currentState);
  justDoCoffee(runningCfg, currentState, brewActive, preinfusionFinished);
}





static void brewDetect(void) {
  static bool paramsReset = true;

  if (brewState() && !stopOnWeight()) {
    openValve();
    if(!paramsReset) {
      brewParamsReset();
      paramsReset = true;
    }
    brewActive = true;
  } else {
    brewActive = false;
    if (startupInitFinished) {
      setPumpOff();
      closeValve();
    }
    if(!brewState() && paramsReset) {
      brewParamsReset();
      paramsReset = false;
    }
  }
}

static void brewParamsReset(void) {
  tareDone = false;
  shotWeight = 0.f;
  brewStopWeight = 0.f;
  previousWeight = 0.f;
  currentState.weight = 0.f;
  currentState.liquidPumped = 0.f;
  currentState.isHeadSpaceFilled = false;
  currentState.isPredictiveWeightForceStarted = false;
  preinfusionFinished = false;
  brewingTimer = millis();
  systemHealthTimer = millis() + HEALTHCHECK_EVERY;
  lcdSendFakeTouch();
}


static void flushActivated(void) {
  #if defined SINGLE_BOARD || defined LEGO_VALVE_RELAY
      openValve();
  #endif
  setPumpFullOn();
}

static void flushDeactivated(void) {
  #if defined SINGLE_BOARD || defined LEGO_VALVE_RELAY
      closeValve();
  #endif
  setPumpOff();
}

static void fillBoiler(float targetBoilerFullPressure) {
  static unsigned long timePassed = millis();
  if (currentState.pressure < targetBoilerFullPressure && !startupInitFinished ) {
    lcdShowPopup("Filling boiler!");
    openValve();
    setPumpToRawValue(80);
    if (millis() - timePassed > 5000UL) startupInitFinished = true;
  } else startupInitFinished = true;
}

static void systemHealthCheck(float pressureThreshold) {
  static int safetyTimer;
  if (!brewState() && !steamState()) {
    if (millis() >= systemHealthTimer) {
      safetyTimer = millis();
      lcdShowPopup("Releasing pressure!");
      while (currentState.smoothedPressure >= pressureThreshold && currentState.temperature < STEAM_WAND_HOT_WATER_TEMP)
      {
        setBoilerOff();
        openValve();
        sensorsRead();
        if (millis() > safetyTimer + 1000) return;
      }
      closeValve();
      systemHealthTimer = millis() + HEALTHCHECK_EVERY;
    }
  }



  while (currentState.temperature <= 0.0f || currentState.temperature == NAN || currentState.temperature >= 170.0f) {


    setBoilerOff();
    if (millis() > thermoTimer) {
      LOG_ERROR("Cannot read temp from thermocouple (last read: %.1lf)!", currentState.temperature);
      lcdShowPopup("TEMP READ ERROR");
      currentState.temperature = thermocouple.readCelsius();
      thermoTimer = millis() + GET_KTYPE_READ_EVERY;
    }
  }
}