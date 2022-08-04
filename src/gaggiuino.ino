#if defined(DEBUG_ENABLED)
  #include "dbg.h"
#endif

#include "gaggiuino.h"
#include "profiling_phases.h"
#include "log.h"
#include "eeprom_data.h"
#include "lcd/lcd.h"
#include "peripherals/pump.h"
#include "peripherals/pressure_sensor.h"
#include "peripherals/thermocouple.h"
#include "peripherals/scales.h"
#include "peripherals/peripherals.h"
#include "sensors_state.h"

// Some vars are better global
//Timers
unsigned long pressureTimer = 0;
unsigned long thermoTimer = 0;
unsigned long scalesTimer = 0;
unsigned long flowTimer = 0;
unsigned long pageRefreshTimer = 0;
unsigned long brewingTimer = 0;

SensorState currentState;

//scales vars
float shotWeight;
float previousWeight;
bool tareDone = false;

// brew detection vars
bool brewActive;

//PP&PI variables
//default phases. Updated in updatePressureProfilePhases.
Phase phaseArray[6];
Phases phases {6,  phaseArray};
int preInfusionFinishedPhaseIdx = 3;
bool preinfusionFinished;

eepromValues_t runningCfg;

OPERATION_MODES selectedOperationalMode;
bool homeScreenScalesEnabled;

// Other util vars
float pressureTargetComparator;

void setup(void) {
  LOG_INIT();
  LOG_INFO("Gaggiuino (fw: %s) booting", AUTO_VERSION);

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

  lcdInit();
  LOG_INFO("LCD Init");

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
}

static void sensorsReadTemperature(void) {
  if (millis() > thermoTimer) {
    currentState.temperature = thermocouple.readCelsius();
    /*
    This *while* is here to prevent situations where the system failed to get a temp reading and temp reads as 0 or -7(cause of the offset)
    If we would use a non blocking function then the system would keep the SSR in HIGH mode which would most definitely cause boiler overheating
    */
    while (currentState.temperature <= 0.0f || currentState.temperature  == NAN || currentState.temperature  > 165.0f) {
      /* In the event of the temp failing to read while the SSR is HIGH
      we force set it to LOW while trying to get a temp reading - IMPORTANT safety feature */
      setBoilerOff();
      if (millis() > thermoTimer) {
        LOG_ERROR("Cannot read temp from thermocouple (last read: %.1lf)!", currentState.temperature );
        lcdWriteLcdMessage("TEMP READ ERROR"); // writing a LCD message
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
      tareDone = true;
    }
    currentState.weight = scalesGetWeight();
    scalesTimer = millis() + GET_SCALES_READ_EVERY;
  }
}

static void sensorsReadPressure(void) {
  if (millis() > pressureTimer) {
    currentState.pressure = getPressure();
    currentState.isPressureFalling = isPressureFalling();
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
      long pumpClicks =  getAndResetClickCounter();
      float cps = 1000.f * pumpClicks / elapsedTime;
      currentState.pumpFlow = getPumpFlow(cps, currentState.pressure);

      if (scalesIsPresent()) {
        currentState.weightFlow = fmaxf(0.f, (shotWeight - previousWeight) * 1000 / elapsedTime);
        previousWeight = shotWeight;
      } else if (preinfusionFinished) {
        shotWeight += currentState.pumpFlow * elapsedTime / 1000;
      }

      flowTimer = millis();
    }
  }
}

// Stops the pump if setting active and dose/weight conditions met
bool stopOnWeight() {
  if(runningCfg.stopOnWeightState && runningCfg.shotStopOnCustomWeight < 1.f) {
    if (shotWeight > runningCfg.shotDose-0.5f ) return true;
    else return false;
  } else if(runningCfg.stopOnWeightState && runningCfg.shotStopOnCustomWeight > 1.f) {
    if (shotWeight > runningCfg.shotStopOnCustomWeight-0.5f) return true;
    else return false;
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
      if (!steamState()) profiling();
      else steamCtrl();
      break;
    case OPMODE_manual:
      manualPressureProfile();
      break;
    case OPMODE_flush:
      setPumpFullOn();
      justDoCoffee();
      break;
    case OPMODE_steam:
      if (!steamState()) {
        setPumpFullOn();
        justDoCoffee();
      } else {
        steamCtrl();
      }
      break;
    case OPMODE_descale:
      deScale();
      break;
    case OPMODE_empty:
      break;
    default:
      pageValuesRefresh(true);
      break;
  }
}

//#############################################################################################
//#########################____NO_OPTIONS_ENABLED_POWER_CONTROL____############################
//#############################################################################################

//delta stuff
inline static float TEMP_DELTA(float d) { return (d*DELTA_RANGE); }


static void justDoCoffee(void) {
  int HPWR_LOW = runningCfg.hpwr / runningCfg.mainDivider;
  static double heaterWave;
  static bool heaterState;
  float BREW_TEMP_DELTA;
  // Calculating the boiler heating power range based on the below input values
  int HPWR_OUT = mapRange(currentState.temperature, runningCfg.setpoint - 10, runningCfg.setpoint, runningCfg.hpwr, HPWR_LOW, 0);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, runningCfg.hpwr);  // limits range of sensor values to HPWR_LOW and HPWR
  BREW_TEMP_DELTA = mapRange(currentState.temperature, runningCfg.setpoint, runningCfg.setpoint + TEMP_DELTA(runningCfg.setpoint), TEMP_DELTA(runningCfg.setpoint), 0, 0);
  BREW_TEMP_DELTA = constrain(BREW_TEMP_DELTA, 0, TEMP_DELTA(runningCfg.setpoint));

  if (brewActive) {
  // Applying the HPWR_OUT variable as part of the relay switching logic
    if (currentState.temperature > runningCfg.setpoint && currentState.temperature < runningCfg.setpoint + 0.25f && !preinfusionFinished ) {
      if (millis() - heaterWave > HPWR_OUT * runningCfg.brewDivider && !heaterState ) {
        setBoilerOff();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_LOW * runningCfg.mainDivider && heaterState ) {
        setBoilerOn();
        heaterState=false;
        heaterWave=millis();
      }
    } else if (currentState.temperature > runningCfg.setpoint - 1.5f && currentState.temperature < runningCfg.setpoint + (runningCfg.brewDeltaState ? BREW_TEMP_DELTA : 0.f) && preinfusionFinished ) {
      if (millis() - heaterWave > runningCfg.hpwr * runningCfg.brewDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > runningCfg.hpwr && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if (runningCfg.brewDeltaState && currentState.temperature >= (runningCfg.setpoint + BREW_TEMP_DELTA) && currentState.temperature <= (runningCfg.setpoint + BREW_TEMP_DELTA + 2.5f)  && preinfusionFinished ) {
      if (millis() - heaterWave > runningCfg.hpwr * runningCfg.mainDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > runningCfg.hpwr && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if(currentState.temperature <= runningCfg.setpoint - 1.5f) {
      setBoilerOn();
    } else {
      setBoilerOff();
    }
  } else { //if brewState == 0
    if (currentState.temperature < ((float)runningCfg.setpoint - 10.00f)) {
      setBoilerOn();
    } else if (currentState.temperature >= ((float)runningCfg.setpoint - 10.00f) && currentState.temperature < ((float)runningCfg.setpoint - 3.00f)) {
      setBoilerOn();
      if (millis() - heaterWave > HPWR_OUT / runningCfg.brewDivider) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if ((currentState.temperature >= ((float)runningCfg.setpoint - 3.00f)) && (currentState.temperature <= ((float)runningCfg.setpoint - 1.00f))) {
      if (millis() - heaterWave > HPWR_OUT / runningCfg.brewDivider && !heaterState) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_OUT / runningCfg.brewDivider && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if ((currentState.temperature >= ((float)runningCfg.setpoint - 0.5f)) && currentState.temperature < (float)runningCfg.setpoint) {
      if (millis() - heaterWave > HPWR_OUT / runningCfg.brewDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_OUT / runningCfg.brewDivider && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else {
      setBoilerOff();
    }
  }
}

//#############################################################################################
//################################____STEAM_POWER_CONTROL____##################################
//#############################################################################################

static void steamCtrl(void) {
    // steam temp control, needs to be aggressive to keep steam pressure acceptable
  if ((currentState.temperature > runningCfg.setpoint - 10.f) && (currentState.temperature <= STEAM_WAND_HOT_WATER_TEMP)) {
    setBoilerOn();
    brewActive ? setPumpFullOn() : setPumpOff();
  }else if ((currentState.pressure <= 9.f) && (currentState.temperature > STEAM_WAND_HOT_WATER_TEMP) && (currentState.temperature <= STEAM_TEMPERATURE)) {
    setBoilerOn();
    brewActive ? setPumpToRawValue(25) : setPumpOff();
  } else {
    setBoilerOff();
  }
}

//#############################################################################################
//################################____LCD_REFRESH_CONTROL___###################################
//#############################################################################################

static void lcdRefresh(void) {

  if (millis() > pageRefreshTimer) {
    /*LCD pressure output, as a measure to beautify the graphs locking the live pressure read for the LCD alone*/
    #ifdef BEAUTIFY_GRAPH
      float beautifiedPressure = fmin(currentState.pressure, pressureTargetComparator + 0.5f);
      lcdSetPressure(
        brewActive
          ? beautifiedPressure * 10.f
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
    } else {
      lcdSetWeight(
        shotWeight > 0.f
          ? currentState.weight
          : 0.0f
      );
    }

    /*LCD flow output*/
    if (lcdCurrentPageId == 1 || lcdCurrentPageId == 2 || lcdCurrentPageId == 8 ) { // no point sending this continuously if on any other screens than brew related ones
      lcdSetFlow(
        currentState.weight > 0.4f // currentState.weight is always zero if scales are not present
          ? currentState.weightFlow * 10.f
          : currentState.pumpFlow * 10.f
      );
    }

#if defined(DEBUG_ENABLED)
    lcdShowDebug(readTempSensor(), getAdsError());
#endif

    /*LCD timer and warmup*/
    if (brewActive) {
      lcdBrewTimerStart(); // nextion timer start
      lcdWarmupStateStop(); // Flaggig warmup notification on Nextion needs to stop (if enabled)
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
      eepromCurrentValues.stopOnWeightState = lcdValues.stopOnWeightState;
      eepromCurrentValues.shotDose = lcdValues.shotDose;
      eepromCurrentValues.shotPreset = lcdValues.shotPreset;
      eepromCurrentValues.shotStopOnCustomWeight = lcdValues.shotStopOnCustomWeight;
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
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

static void deScale(void) {
  static bool blink = true;
  static long timer = millis();
  static int currentCycleRead = lcdGetDescaleCycle();
  static int lastCycleRead = 10;
  static bool descaleFinished = false;
  if (brewState() && !descaleFinished) {
    if (currentCycleRead < lastCycleRead) { // descale in cycles of 5 then wait according to the below conditions
      if (blink == true) { // Logic that switches between modes depending on the $blink value
        setPumpToRawValue(15);
        if (millis() - timer > DESCALE_PHASE1_EVERY) { //set dimmer power to min descale value for 10 sec
          if (currentCycleRead >=100) descaleFinished = true;
          blink = false;
          currentCycleRead = lcdGetDescaleCycle();
          timer = millis();
        }
      } else {
        setPumpToRawValue(30);
        if (millis() - timer > DESCALE_PHASE2_EVERY) { //set dimmer power to max descale value for 20 sec
          blink = true;
          currentCycleRead++;
          if (currentCycleRead<100) lcdSetDescaleCycle(currentCycleRead);
          timer = millis();
        }
      }
    } else {
      setPumpOff();
      if ((millis() - timer) > DESCALE_PHASE3_EVERY) { //nothing for 5 minutes
        if (currentCycleRead*2 < 100) lcdSetDescaleCycle(currentCycleRead*3);
        else {
          lcdSetDescaleCycle(100);
          descaleFinished = true;
        }
        lastCycleRead = currentCycleRead*2;
        timer = millis();
      }
    }
  } else if (brewState() && descaleFinished == true){
    setPumpOff();
    if ((millis() - timer) > 1000) {
      lcdBrewTimerStop();
      lcdShowDescaleFinished();
      timer=millis();
    }
  } else {
    currentCycleRead = 0;
    lastCycleRead = 10;
    descaleFinished = false;
    timer = millis();
  }
  //keeping it at temp
  justDoCoffee();
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

      pressureTargetComparator = preinfusionFinished ? newBarValue : currentState.pressure;
    } else {
      float newFlowValue = phases.phases[currentPhase.phaseIndex].getTarget(currentPhase.timeInPhase);
      float pressureRestriction =  phases.phases[currentPhase.phaseIndex].getRestriction(currentPhase.timeInPhase);
      setPumpFlow(newFlowValue, pressureRestriction, currentState);
    }
  }
  else {
    setPumpToRawValue(0);
  }
  // Keep that water at temp
  justDoCoffee();
}

static void manualPressureProfile(void) {
  int power_reading = lcdGetManualPressurePower();
  setPumpPressure(power_reading, 0.f, currentState);
  justDoCoffee();
}

//#############################################################################################
//###################################____BREW DETECT____#######################################
//#############################################################################################

static void brewDetect(void) {
  static bool paramsReset = true;

  if ( brewState() && !stopOnWeight()) {
    if(!paramsReset) {
      brewParamsReset();
      paramsReset = true;
    }
    openValve();
    brewActive = true;
  } else if ( brewState() && stopOnWeight()) {
    closeValve();
    brewActive = false;
  } else if (!brewState()){
    closeValve();
    brewActive = false;
    if(paramsReset) {
      brewParamsReset();
      paramsReset = false;
    }
  }
}

static void brewParamsReset() {
  tareDone = false;
  shotWeight = 0.f;
  currentState.weight = 0.f;
  previousWeight = 0.f;
  brewingTimer = millis();
  preinfusionFinished = false;
}

