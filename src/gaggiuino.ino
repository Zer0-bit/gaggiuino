#include <EasyNextionLibrary.h>

#if defined(DEBUG_ENABLED)
  #include "dbg.h"
#endif

#include "PressureProfile.h"
#include "log.h"
#include "eeprom_data.h"
#include "peripherals/pump.h"
#include "peripherals/pressure_sensor.h"
#include "peripherals/thermocouple.h"
#include "peripherals/scales.h"
#include "peripherals/peripherals.h"

// Define some const values
#define GET_KTYPE_READ_EVERY    250 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#define GET_PRESSURE_READ_EVERY 10
#define GET_SCALES_READ_EVERY   100
#define REFRESH_SCREEN_EVERY    150 // Screen refresh interval (ms)
#define REFRESH_FLOW_EVERY      1000
#define DESCALE_PHASE1_EVERY    500 // short pump pulses during descale
#define DESCALE_PHASE2_EVERY    5000 // short pause for pulse effficience activation
#define DESCALE_PHASE3_EVERY    120000 // long pause for scale softening
#define DELTA_RANGE             0.25f // % to apply as delta
#define BEAUTIFY_GRAPH

// EasyNextion object init
EasyNex myNex(USART_LCD);

// Some vars are better global
//Timers
unsigned long pressureTimer = 0;
unsigned long thermoTimer = 0;
unsigned long scalesTimer = 0;
unsigned long flowTimer = 0;
unsigned long pageRefreshTimer = 0;
unsigned long brewingTimer = 0;

//volatile vars
volatile float kProbeReadValue; //temp val
volatile float livePressure;
volatile float liveWeight;

//scales vars
float shotWeight;
float currentWeight;
float previousWeight;
float flowVal;
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

int selectedOperationalMode;
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
  LOG_INFO("Valve opened");

  // Will wait hereuntil full serial is established, this is done so the LCD fully initializes before passing the EEPROM values
  USART_LCD.begin(115200);
  while (myNex.readNumber("safetyTempCheck") != 100 )
  {
    LOG_VERBOSE("Connecting to Nextion LCD");
    delay(100);
  }
  myNex.writeStr("splash.build_version.txt", AUTO_VERSION);

  // Initialising the vsaved values or writing defaults if first start
  eepromInit();
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  LOG_INFO("EEPROM Init");

  thermocoupleInit();
  LOG_INFO("Thermocouple Init");

  lcdInit(eepromCurrentValues);
  LOG_INFO("LCD init");

  pressureSensorInit();
  LOG_INFO("Pressure sensor init");

  // Scales handling
  scalesInit(eepromCurrentValues.scalesF1, eepromCurrentValues.scalesF2);
  LOG_INFO("Scales init");

  // Pump init
  pumpInit(eepromCurrentValues.powerLineFrequency);
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
  myNex.NextionListen();
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
    kProbeReadValue = thermocouple.readCelsius();  // Making sure we're getting a value
    /*
    This *while* is here to prevent situations where the system failed to get a temp reading and temp reads as 0 or -7(cause of the offset)
    If we would use a non blocking function then the system would keep the SSR in HIGH mode which would most definitely cause boiler overheating
    */
    while (kProbeReadValue <= 0.0f || kProbeReadValue == NAN || kProbeReadValue > 165.0f) {
      /* In the event of the temp failing to read while the SSR is HIGH
      we force set it to LOW while trying to get a temp reading - IMPORTANT safety feature */
      setBoilerOff();
      if (millis() > thermoTimer) {
        LOG_ERROR("Cannot read temp from thermocouple (last read: %.1lf)!", kProbeReadValue);
        kProbeReadValue = thermocouple.readCelsius();  // Making sure we're getting a value
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
    currentWeight = scalesGetWeight();
    scalesTimer = millis() + GET_SCALES_READ_EVERY;
  }
}

static void sensorsReadPressure(void) {
  if (millis() > pressureTimer) {
    livePressure = getPressure();
    pressureTimer = millis() + GET_PRESSURE_READ_EVERY;
  }
}

static void calculateWeightAndFlow(void) {
  if (brewActive) {
    if (scalesIsPresent()) {
      shotWeight = currentWeight;

      if (millis() > flowTimer) {
        flowVal = shotWeight - previousWeight;
        previousWeight = shotWeight;
        flowTimer = millis() + REFRESH_FLOW_EVERY;
      }
    } else {
      if (millis() > flowTimer) {
        flowVal = getPumpFlow(getAndResetClickCounter(), livePressure) * 1000 / REFRESH_FLOW_EVERY;
        if (preinfusionFinished) {
          currentWeight += flowVal;
          shotWeight = currentWeight;
        }
        flowTimer = millis() + REFRESH_FLOW_EVERY;
      }
    }
  }
}

//##############################################################################################################################
//############################################______PAGE_CHANGE_VALUES_REFRESH_____#############################################
//##############################################################################################################################

static void pageValuesRefresh(bool forcedUpdate) {  // Refreshing our values on page changes

  if ( myNex.currentPageId != myNex.lastCurrentPageId || forcedUpdate == true ) {
    runningCfg.preinfusionState           = myNex.readNumber("piState"); // reding the preinfusion state value which should be 0 or 1
    runningCfg.preinfusionSec             = myNex.readNumber("piSec");
    runningCfg.preinfusionBar             = myNex.readNumber("piBar");
    runningCfg.preinfusionSoak            = myNex.readNumber("piSoak"); // pre-infusion soak value
    runningCfg.preinfusionRamp            = myNex.readNumber("piRamp"); // ramp speed btw PI and PP pressures

    runningCfg.pressureProfilingState     = myNex.readNumber("ppState"); // reding the pressure profile state value which should be 0 or 1
    runningCfg.pressureProfilingStart     = myNex.readNumber("ppStart");
    runningCfg.pressureProfilingFinish    = myNex.readNumber("ppFin");
    runningCfg.pressureProfilingHold      = myNex.readNumber("ppHold"); // pp start pressure hold
    runningCfg.pressureProfilingLength    = myNex.readNumber("ppLength"); // pp shot length

    runningCfg.brewDeltaState             = myNex.readNumber("deltaState");
    runningCfg.setpoint                   = myNex.readNumber("setPoint");  // reading the setPoint value from the lcd
    runningCfg.offsetTemp                 = myNex.readNumber("offSet");  // reading the offset value from the lcd
    runningCfg.hpwr                       = myNex.readNumber("hpwr");  // reading the brew time delay used to apply heating in waves
    runningCfg.mainDivider                = myNex.readNumber("mDiv");  // reading the delay divider
    runningCfg.brewDivider                = myNex.readNumber("bDiv");  // reading the delay divider
    runningCfg.powerLineFrequency         = myNex.readNumber("regHz");
    runningCfg.warmupState                = myNex.readNumber("warmupState");

    homeScreenScalesEnabled = myNex.readNumber("scalesEnabled");
    selectedOperationalMode = myNex.readNumber("modeSelect"); // MODE_SELECT should always be LAST

    updatePressureProfilePhases();

    myNex.lastCurrentPageId = myNex.currentPageId;
  }
}

//#############################################################################################
//############################____OPERATIONAL_MODE_CONTROL____#################################
//#############################################################################################
static void modeSelect(void) {
  // USART_CH1.println("MODE SELECT ENTER");
  switch (selectedOperationalMode) {
    case 0:
    case 1:
    case 2:
    case 4:
      if (!steamState()) newPressureProfile();
      else steamCtrl();
      break;
    case 3:
      // USART_CH1.println("MODE SELECT 3");
      manualPressureProfile();
      break;
    case 5:
      // USART_CH1.println("MODE SELECT 5");
      if (!steamState()) pumpFullOn();
      else steamCtrl();
      break;
    case 6:
      // USART_CH1.println("MODE SELECT 6");
      deScale();
      break;
    case 7:
      // USART_CH1.println("MODE SELECT 7");
      break;
    case 8:
      // USART_CH1.println("MODE SELECT 8");
      break;
    case 9:
      // USART_CH1.println("MODE SELECT 9");
      if (!steamState()) pumpFullOn();
      else steamCtrl();
      break;
    default:
      pageValuesRefresh(true);
      break;
  }
  // USART_CH1.println("MODE SELECT EXIT");
}

//#############################################################################################
//#########################____NO_OPTIONS_ENABLED_POWER_CONTROL____############################
//#############################################################################################

//delta stuff
inline static float TEMP_DELTA(float d) { return (d*DELTA_RANGE); }


static void justDoCoffee(void) {
  // USART_CH1.println("DO_COFFEE ENTER");
  int HPWR_LOW = runningCfg.hpwr / runningCfg.mainDivider;
  static double heaterWave;
  static bool heaterState;
  float BREW_TEMP_DELTA;
  // Calculating the boiler heating power range based on the below input values
  int HPWR_OUT = mapRange(kProbeReadValue, runningCfg.setpoint - 10, runningCfg.setpoint, runningCfg.hpwr, HPWR_LOW, 0);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, runningCfg.hpwr);  // limits range of sensor values to HPWR_LOW and HPWR
  BREW_TEMP_DELTA = mapRange(kProbeReadValue, runningCfg.setpoint, runningCfg.setpoint + TEMP_DELTA(runningCfg.setpoint), TEMP_DELTA(runningCfg.setpoint), 0, 0);
  BREW_TEMP_DELTA = constrain(BREW_TEMP_DELTA, 0, TEMP_DELTA(runningCfg.setpoint));

  // USART_CH1.println("DO_COFFEE TEMP CTRL BEGIN");
  if (brewActive) {
  // Applying the HPWR_OUT variable as part of the relay switching logic
    if (kProbeReadValue > runningCfg.setpoint && kProbeReadValue < runningCfg.setpoint + 0.25f && !preinfusionFinished ) {
      if (millis() - heaterWave > HPWR_OUT * runningCfg.brewDivider && !heaterState ) {
        setBoilerOff();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_LOW * runningCfg.mainDivider && heaterState ) {
        setBoilerOn();
        heaterState=false;
        heaterWave=millis();
      }
    } else if (kProbeReadValue > runningCfg.setpoint - 1.5f && kProbeReadValue < runningCfg.setpoint + (runningCfg.brewDeltaState ? BREW_TEMP_DELTA : 0.f) && preinfusionFinished ) {
      if (millis() - heaterWave > runningCfg.hpwr * runningCfg.brewDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > runningCfg.hpwr && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if (runningCfg.brewDeltaState && kProbeReadValue >= (runningCfg.setpoint + BREW_TEMP_DELTA) && kProbeReadValue <= (runningCfg.setpoint + BREW_TEMP_DELTA + 2.5f)  && preinfusionFinished ) {
      if (millis() - heaterWave > runningCfg.hpwr * runningCfg.mainDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > runningCfg.hpwr && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if(kProbeReadValue <= runningCfg.setpoint - 1.5f) {
      setBoilerOn();
    } else {
      setBoilerOff();
    }
  } else { //if brewState == 0
    if (kProbeReadValue < ((float)runningCfg.setpoint - 10.00f)) {
      setBoilerOn();
    } else if (kProbeReadValue >= ((float)runningCfg.setpoint - 10.00f) && kProbeReadValue < ((float)runningCfg.setpoint - 3.00f)) {
      setBoilerOn();
      if (millis() - heaterWave > HPWR_OUT / runningCfg.brewDivider) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if ((kProbeReadValue >= ((float)runningCfg.setpoint - 3.00f)) && (kProbeReadValue <= ((float)runningCfg.setpoint - 1.00f))) {
      if (millis() - heaterWave > HPWR_OUT / runningCfg.brewDivider && !heaterState) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_OUT / runningCfg.brewDivider && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if ((kProbeReadValue >= ((float)runningCfg.setpoint - 0.5f)) && kProbeReadValue < (float)runningCfg.setpoint) {
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

  if (!brewActive) {
    if (livePressure <= 9.f) { // steam temp control, needs to be aggressive to keep steam pressure acceptable
      if ((kProbeReadValue > runningCfg.setpoint - 10.f) && (kProbeReadValue <= 155.f)) setBoilerOn();
      else setBoilerOff();
    } else if(livePressure >= 9.1f) setBoilerOff();
  } else if (brewActive) { //added to cater for hot water from steam wand functionality
    if ((kProbeReadValue > runningCfg.setpoint - 10.f) && (kProbeReadValue <= 105.f)) {
      setBoilerOn();
      pumpFullOn();
    } else {
      setBoilerOff();
      pumpFullOn();
    }
  } else setBoilerOff();
}

//#############################################################################################
//################################____LCD_REFRESH_CONTROL___###################################
//#############################################################################################

static void lcdRefresh(void) {

  if (millis() > pageRefreshTimer) {
    /*LCD pressure output, as a measure to beautify the graphs locking the live pressure read for the LCD alone*/
    #ifdef BEAUTIFY_GRAPH
      myNex.writeNum("pressure.val", (livePressure > 0.f) ? (livePressure <= pressureTargetComparator + 0.5f) ? livePressure*10.f : pressureTargetComparator*10.f : 0.f);
    #else
      myNex.writeNum("pressure.val", (livePressure > 0.f) ? livePressure*10.f : 0.f);
    #endif

    /*LCD temp output*/
    myNex.writeNum("currentTemp",int(kProbeReadValue - runningCfg.offsetTemp));

    /*LCD weight output*/
    if (myNex.currentPageId == 0 && homeScreenScalesEnabled) {
      myNex.writeStr("weight.txt", (currentWeight > 0.f) ? String(currentWeight,1) : "0.0");
    } else {
      myNex.writeStr("weight.txt", (currentWeight > 0.f) ? String(shotWeight,1) : "0.0");
    }

    /*LCD flow output*/
    myNex.writeNum("flow.val", int((flowVal>0.f) ? flowVal * 10 : 0));

    #if defined(DEBUG_ENABLED)
    myNex.writeNum("debug1",readTempSensor());
    myNex.writeNum("debug2",getAdsError());
    #endif

    /*LCD timer and warmup*/
    if (brewActive) {
      brewTimer(1); // nextion timer start
      myNex.writeNum("warmupState", 0); // Flaggig warmup notification on Nextion needs to stop (if enabled)
    } else {
      brewTimer(0); // nextion timer start
    }

    pageRefreshTimer = millis() + REFRESH_SCREEN_EVERY;
  }
}
//#############################################################################################
//###################################____SAVE_BUTTON____#######################################
//#############################################################################################
// Save the desired temp values to EEPROM
void trigger1(void) {
  LOG_VERBOSE("Saving values to EEPROM");
  bool rc;
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();

  switch (myNex.currentPageId){
    case 1:
      break;
    case 2:
      break;
    case 3:
      eepromCurrentValues.pressureProfilingStart    = myNex.readNumber("ppStart");
      eepromCurrentValues.pressureProfilingFinish   = myNex.readNumber("ppFin");
      eepromCurrentValues.pressureProfilingHold     = myNex.readNumber("ppHold");
      eepromCurrentValues.pressureProfilingLength   = myNex.readNumber("ppLength");
      eepromCurrentValues.pressureProfilingState    = myNex.readNumber("ppState");
      eepromCurrentValues.preinfusionState          = myNex.readNumber("piState");
      eepromCurrentValues.preinfusionSec            = myNex.readNumber("piSec");
      eepromCurrentValues.preinfusionBar            = myNex.readNumber("piBar");
      eepromCurrentValues.preinfusionSoak           = myNex.readNumber("piSoak");
      eepromCurrentValues.preinfusionRamp           = myNex.readNumber("piRamp");
      break;
    case 4:
      eepromCurrentValues.homeOnShotFinish  = myNex.readNumber("homeOnBrewFinish");
      eepromCurrentValues.graphBrew         = myNex.readNumber("graphEnabled");
      eepromCurrentValues.brewDeltaState    = myNex.readNumber("deltaState");
      eepromCurrentValues.warmupState       = myNex.readNumber("warmupState");
      break;
    case 5:
      break;
    case 6:
      eepromCurrentValues.setpoint    = myNex.readNumber("setPoint");
      eepromCurrentValues.offsetTemp  = myNex.readNumber("offSet");
      eepromCurrentValues.hpwr        = myNex.readNumber("hpwr");
      eepromCurrentValues.mainDivider = myNex.readNumber("mDiv");
      eepromCurrentValues.brewDivider = myNex.readNumber("bDiv");
      break;
    case 7:
      eepromCurrentValues.powerLineFrequency = myNex.readNumber("regHz");
      eepromCurrentValues.lcdSleep           = myNex.readNumber("systemSleepTime");
      break;
    default:
      break;
  }

  rc = eepromWrite(eepromCurrentValues);
  if (rc == true) {
    myNex.writeStr("popupMSG.t0.txt","UPDATE SUCCESSFUL!");
  } else {
    myNex.writeStr("popupMSG.t0.txt","ERROR!");
  }
  myNex.writeStr("page popupMSG");
}

//#############################################################################################
//###################################_____SCALES_TARE____######################################
//#############################################################################################

void trigger2(void) {
  LOG_VERBOSE("Tare scales");
  if (scalesIsPresent()) scalesTare();
}

void trigger3(void) {
  LOG_VERBOSE("Scales enabled or disabled");
  homeScreenScalesEnabled = myNex.readNumber("scalesEnabled");
}

//#############################################################################################
//###############################_____HELPER_FUCTIONS____######################################
//#############################################################################################

static void brewTimer(bool c) { // small function for easier timer start/stop
  myNex.writeNum("timerState", c ? 1 : 0);
}


//#############################################################################################
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

static void deScale(void) {
  static bool blink = true;
  static long timer = millis();
  static int currentCycleRead = myNex.readNumber("j0.val");
  static int lastCycleRead = 10;
  static bool descaleFinished = false;
  if (brewActive && !descaleFinished) {
    if (currentCycleRead < lastCycleRead) { // descale in cycles for 5 times then wait according to the below condition
      if (blink == true) { // Logic that switches between modes depending on the $blink value
        setPumpToRawValue(15);
        if (millis() - timer > DESCALE_PHASE1_EVERY) { //set dimmer power to max descale value for 10 sec
          if (currentCycleRead >=100) descaleFinished = true;
          blink = false;
          currentCycleRead = myNex.readNumber("j0.val");
          timer = millis();
        }
      } else {
        setPumpToRawValue(30);
        if (millis() - timer > DESCALE_PHASE2_EVERY) { //set dimmer power to min descale value for 20 sec
          blink = true;
          currentCycleRead++;
          if (currentCycleRead<100) myNex.writeNum("j0.val", currentCycleRead);
          timer = millis();
        }
      }
    } else {
      setPumpOff();
      if ((millis() - timer) > DESCALE_PHASE3_EVERY) { //nothing for 5 minutes
        if (currentCycleRead*3 < 100) myNex.writeNum("j0.val", currentCycleRead*3);
        else {
          myNex.writeNum("j0.val", 100);
          descaleFinished = true;
        }
        lastCycleRead = currentCycleRead*3;
        timer = millis();
      }
    }
  } else if (brewActive && descaleFinished == true){
    setPumpOff();
    if ((millis() - timer) > 1000) {
      brewTimer(0);
      myNex.writeStr("t14.txt", "FINISHED!");
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
  switch (selectedOperationalMode)
  {
  case 0: // no PI and no PP -> Pressure fixed at 9bar
    phases.count = 1;
    setPhase(0, 9, 9, 1000);
    preInfusionFinishedPhaseIdx = 0;
    break;
  case 1: // Just PI no PP -> after PI pressure fixed at 9bar
    phases.count = 4;
    setPreInfusionPhases(0, runningCfg.preinfusionBar, runningCfg.preinfusionSec, runningCfg.preinfusionSoak);
    setPhase(3, runningCfg.preinfusionBar, 9, runningCfg.preinfusionRamp*1000);
    preInfusionFinishedPhaseIdx = 3;
    break;
  case 2: // No PI, yes PP
    phases.count = 2;
    setPhase(3, 0, runningCfg.pressureProfilingStart, runningCfg.preinfusionRamp*1000);
    setPresureProfilePhases(0, runningCfg.pressureProfilingStart, runningCfg.pressureProfilingFinish, runningCfg.pressureProfilingHold, runningCfg.pressureProfilingLength);
    preInfusionFinishedPhaseIdx = 0;
    break;
  case 4: // Both PI + PP
    phases.count = 6;
    setPreInfusionPhases(0, runningCfg.preinfusionBar, runningCfg.preinfusionSec, runningCfg.preinfusionSoak);
    setPhase(3, runningCfg.preinfusionBar, runningCfg.pressureProfilingStart, runningCfg.preinfusionRamp*1000);
    setPresureProfilePhases(4, runningCfg.pressureProfilingStart, runningCfg.pressureProfilingFinish, runningCfg.pressureProfilingHold, runningCfg.pressureProfilingLength);
    preInfusionFinishedPhaseIdx = 3;
    break;
  default:
    break;
  }
}

void setPreInfusionPhases(int startingIdx, int piBar, int piTime, int piSoakTime) {
    setPhase(startingIdx + 0, piBar/2, piBar, piTime * 1000 / 2);
    setPhase(startingIdx + 1, piBar, piBar, piTime * 1000 / 2);
    setPhase(startingIdx + 2, 0, 0, piSoakTime * 1000);
}

void setPresureProfilePhases(int startingIdx,int fromBar, int toBar, int holdTime, int dropTime) {
    setPhase(startingIdx + 0, fromBar, fromBar, holdTime * 1000);
    setPhase(startingIdx + 1, fromBar, toBar, dropTime * 1000);
}

void setPhase(int phaseIdx, int fromBar, int toBar, int timeMs) {
    phases.phases[phaseIdx].startPressure = fromBar;
    phases.phases[phaseIdx].endPressure = toBar;
    phases.phases[phaseIdx].durationMs = timeMs;
}

static void newPressureProfile(void) {
  float newBarValue;

  if (brewActive) { //runs this only when brew button activated and pressure profile selected
    long timeInPP = millis() - brewingTimer;
    CurrentPhase currentPhase = phases.getCurrentPhase(timeInPP);
    newBarValue = phases.phases[currentPhase.phaseIndex].getPressure(currentPhase.timeInPhase);
    preinfusionFinished = currentPhase.phaseIndex >= preInfusionFinishedPhaseIdx;
  }
  else {
    newBarValue = 0.0f;
  }
  setPumpPressure(livePressure, newBarValue, flowVal, isPressureFalling());
  // saving the target pressure
  pressureTargetComparator = preinfusionFinished ? newBarValue : livePressure;
  // Keep that water at temp
  justDoCoffee();
}

static void manualPressureProfile(void) {
  if( selectedOperationalMode == 3 ) {
    int power_reading = myNex.readNumber("h0.val");
    setPumpPressure(livePressure, power_reading, flowVal, isPressureFalling());
  }
  justDoCoffee();
}

static void pumpFullOn(void) {
  setPumpToRawValue(PUMP_RANGE);
  justDoCoffee();
}

//#############################################################################################
//###################################____BREW DETECT____#######################################
//#############################################################################################

static void brewDetect(void) {
  if ( brewState() ) {
    openValve();
    if (!brewActive) {
      brewJustStarted();
    }
    brewActive = true;
  } else{
    closeValve();
    brewActive = false;
  }
}

static void brewJustStarted() {
  tareDone = false;
  shotWeight = 0.f;
  currentWeight = 0.f;
  previousWeight = 0.f;
  brewingTimer = millis();
  preinfusionFinished = false;
}

static void lcdInit(eepromValues_t eepromCurrentValues) {

  myNex.writeNum("setPoint", eepromCurrentValues.setpoint);
  myNex.writeNum("moreTemp.n1.val", eepromCurrentValues.setpoint);

  myNex.writeNum("offSet", eepromCurrentValues.offsetTemp);
  myNex.writeNum("moreTemp.n2.val", eepromCurrentValues.offsetTemp);

  myNex.writeNum("hpwr", eepromCurrentValues.hpwr);
  myNex.writeNum("moreTemp.n3.val", eepromCurrentValues.hpwr);

  myNex.writeNum("mDiv", eepromCurrentValues.mainDivider);
  myNex.writeNum("moreTemp.n4.val", eepromCurrentValues.mainDivider);

  myNex.writeNum("bDiv", eepromCurrentValues.brewDivider);
  myNex.writeNum("moreTemp.n5.val", eepromCurrentValues.brewDivider);

  myNex.writeNum("ppStart", eepromCurrentValues.pressureProfilingStart);
  myNex.writeNum("brewAuto.n2.val", eepromCurrentValues.pressureProfilingStart);

  myNex.writeNum("ppFin", eepromCurrentValues.pressureProfilingFinish);
  myNex.writeNum("brewAuto.n3.val", eepromCurrentValues.pressureProfilingFinish);

  myNex.writeNum("ppHold", eepromCurrentValues.pressureProfilingHold);
  myNex.writeNum("brewAuto.n5.val", eepromCurrentValues.pressureProfilingHold);

  myNex.writeNum("ppLength", eepromCurrentValues.pressureProfilingLength);
  myNex.writeNum("brewAuto.n6.val", eepromCurrentValues.pressureProfilingLength);

  myNex.writeNum("piState", eepromCurrentValues.preinfusionState);
  myNex.writeNum("brewAuto.bt0.val", eepromCurrentValues.preinfusionState);

  myNex.writeNum("ppState", eepromCurrentValues.pressureProfilingState);
  myNex.writeNum("brewAuto.bt1.val", eepromCurrentValues.pressureProfilingState);

  myNex.writeNum("piSec", eepromCurrentValues.preinfusionSec);
  myNex.writeNum("brewAuto.n0.val", eepromCurrentValues.preinfusionSec);

  myNex.writeNum("piBar", eepromCurrentValues.preinfusionBar);
  myNex.writeNum("brewAuto.n1.val", eepromCurrentValues.preinfusionBar);

  myNex.writeNum("piSoak", eepromCurrentValues.preinfusionSoak);
  myNex.writeNum("brewAuto.n4.val", eepromCurrentValues.preinfusionSoak);

  myNex.writeNum("piRamp", eepromCurrentValues.preinfusionRamp);
  myNex.writeNum("brewAuto.rampSpeed.val", eepromCurrentValues.preinfusionRamp);

  myNex.writeNum("regHz", eepromCurrentValues.powerLineFrequency);

  myNex.writeNum("systemSleepTime", eepromCurrentValues.lcdSleep);
  myNex.writeNum("morePower.n1.val", eepromCurrentValues.lcdSleep);

  myNex.writeNum("homeOnBrewFinish", eepromCurrentValues.homeOnShotFinish);
  myNex.writeNum("brewSettings.btGoHome.val", eepromCurrentValues.homeOnShotFinish);

  myNex.writeNum("graphEnabled", eepromCurrentValues.graphBrew);
  myNex.writeNum("brewSettings.btGraph.val", eepromCurrentValues.graphBrew);

  myNex.writeNum("warmupState", eepromCurrentValues.warmupState);
  myNex.writeNum("brewSettings.btWarmup.val", eepromCurrentValues.warmupState);

  myNex.writeNum("deltaState", eepromCurrentValues.brewDeltaState);
  myNex.writeNum("brewSettings.btTempDelta.val", eepromCurrentValues.brewDeltaState);
}


