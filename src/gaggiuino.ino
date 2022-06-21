#include <EasyNextionLibrary.h>
#if defined(MAX31855_ENABLED)
  #include <Adafruit_MAX31855.h>
#else
  #include <max6675.h>
#endif
#if defined(SINGLE_HX711_CLOCK)
  #include <HX711_2.h>
#else
  #include <HX711.h>
#endif

#if defined(DEBUG_ENABLED)
  #include "dbg.h"
#endif

#include "PressureProfile.h"
#include "log.h"
#include "eeprom_data.h"
#include "peripherals/pump.h"
#include "peripherals/pressure_sensor.h"
#include "peripherals/peripherals.h"

// Define some const values
#define GET_KTYPE_READ_EVERY 250 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#define GET_PRESSURE_READ_EVERY 50
#define GET_SCALES_READ_EVERY 100
#define REFRESH_SCREEN_EVERY 150 // Screen refresh interval (ms)
#define DESCALE_PHASE1_EVERY 500 // short pump pulses during descale
#define DESCALE_PHASE2_EVERY 5000 // short pause for pulse effficience activation
#define DESCALE_PHASE3_EVERY 120000 // long pause for scale softening
#define MAX_SETPOINT_VALUE 110 //Defines the max value of the setpoint

//Init the thermocouples with the appropriate pins defined above with the prefix "thermo"
#if defined(MAX31855_ENABLED)
  Adafruit_MAX31855 thermocouple(thermoCLK, thermoCS, thermoDO);
#else
  MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
#endif
// EasyNextion object init
EasyNex myNex(USART_LCD);
//#######################__HX711_stuff__##################################
#if defined(SINGLE_HX711_CLOCK)
HX711_2 LoadCells;
#else
HX711 LoadCell_1; //HX711 1
HX711 LoadCell_2; //HX711 2
#endif


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
/* If building for STM32 define the scales factors here */
float scalesF1 = -4183.14f; // 3,911.142856
float scalesF2 = 3911.14f; // -4,183.142856
float currentWeight;
float previousWeight;
float flowVal;

//int tarcalculateWeight;
bool weighingStartRequested;
bool scalesPresent;
bool tareDone;

// brew detection vars
bool brewActive;
bool previousBrewState;

//PP&PI variables
//default phases. Updated in updatePressureProfilePhases.
Phase phaseArray[] = {
  Phase{1, 2, 6000},
  Phase{2, 2, 6000},
  Phase{0, 0, 2000},
  Phase{9, 9, 500},
  Phase{9, 6, 40000}
};
Phases phases {5,  phaseArray};
int preInfusionFinishedPhaseIdx = 3;

bool preinfusionFinished;

bool POWER_ON;
bool  descaleCheckBox;
bool  preinfusionState;
bool  pressureProfileState;
bool  warmupEnabled;
bool  flushEnabled;
bool  descaleEnabled;
bool brewDeltaActive;
bool homeScreenScalesEnabled;
volatile int  HPWR;
volatile int  HPWR_OUT;
int  setPoint;
int  offsetTemp;
int  MainCycleDivider;
int  BrewCycleDivider;
int  preinfuseTime;
int preinfuseBar;
int preinfuseSoak;
int ppStartBar;
int ppFinishBar;
int ppHold;
int ppLength;
int selectedOperationalMode;
int regionHz;

// Other util vars
float pressureTargetComparator;

void setup() {
  LOG_INIT();
  LOG_INFO("Gaggiuino booting");

  // Various pins operation mode handling
  pinInit();
  LOG_INFO("Pin init");

  // Debug init if enabled
  dbgInit();
  LOG_INFO("DBG init");

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

  // Initialising the vsaved values or writing defaults if first start
  eepromInit();
  LOG_INFO("EEPROM Init");

  #if defined(MAX31855_ENABLED)
    thermocouple.begin();
  #endif

  lcdInit();
  LOG_INFO("LCD init");

  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();
  scalesF1 = eepromCurrentValues.scalesF1;
  scalesF2 = eepromCurrentValues.scalesF2;
  setPoint = eepromCurrentValues.setpoint;
  preinfuseSoak = eepromCurrentValues.preinfusionSoak;

  pressureSensorInit();
  LOG_INFO("Pressure sensor init");

  // Scales handling
  scalesInit();
  LOG_INFO("Scales init");

  myNex.lastCurrentPageId = myNex.currentPageId;
  POWER_ON = true;
  LOG_INFO("Setup sequence finished");
}

//##############################################################################################################################
//############################################________________MAIN______________################################################
//##############################################################################################################################


//Main loop where all the logic is continuously run
void loop() {
  pageValuesRefresh();
  myNex.NextionListen();
  sensorsRead();
  brewDetect();
  modeSelect();
  lcdRefresh();
}

//##############################################################################################################################
//#############################################___________SENSORS_READ________##################################################
//##############################################################################################################################


void sensorsRead() { // Reading the thermocouple temperature
  // static long thermoTimer;
  // Reading the temperature every 350ms between the loops
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

  // Read pressure and store in a global var for further controls
  if (millis() > pressureTimer) {
    livePressure = getPressure();
    pressureTimer = millis() + GET_PRESSURE_READ_EVERY;
  }
}

void calculateWeight() {
  // static long scalesTimer;

  scalesTare(); //Tare at the start of any weighing cycle

  // Weight output
  if (millis() > scalesTimer) {
    if (scalesPresent && weighingStartRequested) {
      #if defined(SINGLE_HX711_CLOCK)
        if (LoadCells.is_ready()) {
          float values[2];
          LoadCells.get_units(values);
          currentWeight = values[0] + values[1];
        }
      #else
        currentWeight = LoadCell_1.get_units() + LoadCell_2.get_units();
      #endif
    }
    scalesTimer = millis() + GET_SCALES_READ_EVERY;
  }
  calculateFlow();
}

void calculateFlow() {
  // static long refreshTimer;

  if (millis() >= flowTimer) {
    flowVal = (currentWeight - previousWeight)*10;
    previousWeight = currentWeight;
    flowTimer = millis() + 1000;
  }
}

//##############################################################################################################################
//############################################______PAGE_CHANGE_VALUES_REFRESH_____#############################################
//##############################################################################################################################

void pageValuesRefresh() {  // Refreshing our values on page changes

  if ( myNex.currentPageId != myNex.lastCurrentPageId || POWER_ON == true ) {
    preinfusionState        = myNex.readNumber("piState"); // reding the preinfusion state value which should be 0 or 1
    pressureProfileState    = myNex.readNumber("ppState"); // reding the pressure profile state value which should be 0 or 1
    preinfuseTime           = myNex.readNumber("piSec");
    preinfuseBar            = myNex.readNumber("piBar");
    preinfuseSoak           = myNex.readNumber("piSoak"); // pre-infusion soak value
    ppStartBar              = myNex.readNumber("ppStart");
    ppFinishBar             = myNex.readNumber("ppFin");
    ppHold                  = myNex.readNumber("ppHold"); // pp start pressure hold
    ppLength                = myNex.readNumber("ppLength"); // pp shot length
    brewDeltaActive         = myNex.readNumber("deltaState");
    flushEnabled            = myNex.readNumber("flushState");
    descaleEnabled          = myNex.readNumber("descaleState");
    setPoint                = myNex.readNumber("setPoint");  // reading the setPoint value from the lcd
    offsetTemp              = myNex.readNumber("offSet");  // reading the offset value from the lcd
    HPWR                    = myNex.readNumber("hpwr");  // reading the brew time delay used to apply heating in waves
    MainCycleDivider        = myNex.readNumber("mDiv");  // reading the delay divider
    BrewCycleDivider        = myNex.readNumber("bDiv");  // reading the delay divider
    regionHz                = myNex.readNumber("regHz");
    warmupEnabled           = myNex.readNumber("warmupState");
    homeScreenScalesEnabled = myNex.readNumber("scalesEnabled");

    // MODE_SELECT should always be last
    selectedOperationalMode = myNex.readNumber("modeSelect");

    updatePressureProfilePhases();

    myNex.lastCurrentPageId = myNex.currentPageId;
    POWER_ON = false;
  }
}

//#############################################################################################
//############################____OPERATIONAL_MODE_CONTROL____#################################
//#############################################################################################
void modeSelect() {
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
      if (!steamState() ) justDoCoffee();
      else steamCtrl();
      break;
    case 6:
      // USART_CH1.println("MODE SELECT 6");
      deScale(descaleCheckBox);
      break;
    case 7:
      // USART_CH1.println("MODE SELECT 7");
      break;
    case 8:
      // USART_CH1.println("MODE SELECT 8");
      break;
    case 9:
      // USART_CH1.println("MODE SELECT 9");
      if (!steamState() ) justDoCoffee();
      else steamCtrl();
      break;
    default:
      POWER_ON = true;
      pageValuesRefresh();
      break;
  }
  // USART_CH1.println("MODE SELECT EXIT");
}

//#############################################################################################
//#########################____NO_OPTIONS_ENABLED_POWER_CONTROL____############################
//#############################################################################################
void justDoCoffee() {
  // USART_CH1.println("DO_COFFEE ENTER");
  int HPWR_LOW = HPWR/MainCycleDivider;
  static double heaterWave;
  static bool heaterState;
  float BREW_TEMP_DELTA;
  // Calculating the boiler heating power range based on the below input values
  HPWR_OUT = mapRange(kProbeReadValue, setPoint - 10, setPoint, HPWR, HPWR_LOW, 0);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, HPWR);  // limits range of sensor values to HPWR_LOW and HPWR
  BREW_TEMP_DELTA = mapRange(kProbeReadValue, setPoint, setPoint+setPoint*0.10, setPoint*0.10f, 0, 0);
  BREW_TEMP_DELTA = constrain(BREW_TEMP_DELTA, 0,  setPoint*0.10f);

  // USART_CH1.println("DO_COFFEE TEMP CTRL BEGIN");
  if (brewActive) {
  // Applying the HPWR_OUT variable as part of the relay switching logic
    if (kProbeReadValue > setPoint-1.5f && kProbeReadValue < setPoint+0.25f && !preinfusionFinished ) {
      if (millis() - heaterWave > HPWR_OUT*BrewCycleDivider && !heaterState ) {
        setBoilerOff();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_LOW*MainCycleDivider && heaterState ) {
        setBoilerOn();
        heaterState=false;
        heaterWave=millis();
      }
    } else if (kProbeReadValue > setPoint-1.5f && kProbeReadValue < setPoint+(brewDeltaActive ? BREW_TEMP_DELTA : 0.f) && preinfusionFinished ) {
      if (millis() - heaterWave > HPWR*BrewCycleDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if (brewDeltaActive && kProbeReadValue >= (setPoint+BREW_TEMP_DELTA) && kProbeReadValue <= (setPoint+BREW_TEMP_DELTA+2.5f)  && preinfusionFinished ) {
      if (millis() - heaterWave > HPWR*MainCycleDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if(kProbeReadValue <= setPoint-1.5f) {
    setBoilerOn();
    } else {
      setBoilerOff();
    }
  } else { //if brewState == 0
    // USART_CH1.println("DO_COFFEE BREW BTN NOT ACTIVE BLOCK");
    //brewTimer(0);
    if (kProbeReadValue < ((float)setPoint - 10.00f)) {
      setBoilerOn();
    } else if (kProbeReadValue >= ((float)setPoint - 10.00f) && kProbeReadValue < ((float)setPoint - 3.00f)) {
      setBoilerOn();
      if (millis() - heaterWave > HPWR_OUT/BrewCycleDivider) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if ((kProbeReadValue >= ((float)setPoint - 3.00f)) && (kProbeReadValue <= ((float)setPoint - 1.00f))) {
      if (millis() - heaterWave > HPWR_OUT/BrewCycleDivider && !heaterState) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_OUT/BrewCycleDivider && heaterState ) {
        setBoilerOff();
        heaterState=false;
        heaterWave=millis();
      }
    } else if ((kProbeReadValue >= ((float)setPoint - 0.5f)) && kProbeReadValue < (float)setPoint) {
      if (millis() - heaterWave > HPWR_OUT/BrewCycleDivider && !heaterState ) {
        setBoilerOn();
        heaterState=true;
        heaterWave=millis();
      } else if (millis() - heaterWave > HPWR_OUT/BrewCycleDivider && heaterState ) {
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

void steamCtrl() {

  if (!brewActive) {
    if (livePressure <= 9.f) { // steam temp control, needs to be aggressive to keep steam pressure acceptable
      if ((kProbeReadValue > setPoint-10.f) && (kProbeReadValue <= 155.f)) setBoilerOn();
      else setBoilerOff();
    } else if(livePressure >= 9.1f) setBoilerOff();
  } else if (brewActive) { //added to cater for hot water from steam wand functionality
    if ((kProbeReadValue > setPoint-10.f) && (kProbeReadValue <= 105.f)) {
      setBoilerOn();
      setPumpPressure(livePressure, 9);
    } else {
      setBoilerOff();
      setPumpPressure(livePressure, 9);
    }
  } else setBoilerOff();
}

//#############################################################################################
//################################____LCD_REFRESH_CONTROL___###################################
//#############################################################################################

void lcdRefresh() {
  // static long pageRefreshTimer;
  static float shotWeight;

  if (millis() > pageRefreshTimer) {
    /*LCD pressure output, as a measure to beautify the graphs locking the live pressure read for the LCD alone*/
    // if (brewActive) myNex.writeNum("pressure.val", (livePressure > 0.f) ? livePressure*10.f : 0.f);
    if (brewActive) myNex.writeNum("pressure.val", (livePressure > 0.f) ? (livePressure <= pressureTargetComparator + 0.5f) ? livePressure*10.f : pressureTargetComparator*10.f : 0.f);
    else myNex.writeNum("pressure.val", (livePressure > 0.f) ? livePressure*10.f : 0.f);
    /*LCD temp output*/
    myNex.writeNum("currentTemp",int(kProbeReadValue-offsetTemp));
    /*LCD weight output*/
    if (weighingStartRequested && brewActive) {
      (currentWeight > 0.f) ? myNex.writeStr("weight.txt",String(currentWeight,1)) : myNex.writeStr("weight.txt", "0.0");
      shotWeight = currentWeight;
    } else if (weighingStartRequested && !brewActive) {
      if (myNex.currentPageId != 0 && !homeScreenScalesEnabled) myNex.writeStr("weight.txt",String(shotWeight,1));
      else if(myNex.currentPageId == 0 && homeScreenScalesEnabled) (currentWeight > 0.f) ? myNex.writeStr("weight.txt",String(currentWeight,1)) : myNex.writeStr("weight.txt", "0.0");
    }
    /*LCD flow output*/
    if (weighingStartRequested) (flowVal>0.f) ? myNex.writeNum("flow.val", int(flowVal)) : myNex.writeNum("flow.val", 0.f);

    dbgOutput();

    pageRefreshTimer = millis() + REFRESH_SCREEN_EVERY;
  }
}
//#############################################################################################
//###################################____SAVE_BUTTON____#######################################
//#############################################################################################
// Save the desired temp values to EEPROM
void trigger1() {
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
      eepromCurrentValues.pressureProfilingState  = myNex.readNumber("ppState");
      eepromCurrentValues.preinfusionState    = myNex.readNumber("piState");
      eepromCurrentValues.preinfusionSec      = myNex.readNumber("piSec");
      eepromCurrentValues.preinfusionBar      = myNex.readNumber("piBar");
      eepromCurrentValues.preinfusionSoak     = myNex.readNumber("piSoak");
      break;
    case 4:
      eepromCurrentValues.homeOnShotFinish  = myNex.readNumber("homeOnBrewFinish");
      eepromCurrentValues.graphBrew         = myNex.readNumber("graphEnabled");
      eepromCurrentValues.brewDeltaState    = myNex.readNumber("deltaState");
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
      eepromCurrentValues.powerLineFrequency = myNex.readNumber("regHz");
      eepromCurrentValues.warmupState = myNex.readNumber("warmupState");
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

void trigger2() {
  LOG_VERBOSE("Tare scales");
  tareDone = false;
  previousBrewState = false;
  scalesTare();
}

void trigger3() {
  LOG_VERBOSE("Scales enabled or disabled");
  homeScreenScalesEnabled = myNex.readNumber("scalesEnabled");
}

//#############################################################################################
//###############################_____HELPER_FUCTIONS____######################################
//#############################################################################################

void brewTimer(bool c) { // small function for easier timer start/stop
  myNex.writeNum("timerState", c ? 1 : 0);
}


//#############################################################################################
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

void deScale(bool c) {
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
void updatePressureProfilePhases() {
  switch (selectedOperationalMode)
  {
  case 0: // no PI and no PP -> Pressure fixed at 9bar
    phases.count = 1;
    setPhase(0, 9, 9, 1000);
    preInfusionFinishedPhaseIdx = 0;
    break;
  case 1: // Just PI no PP -> after PI pressure fixed at 9bar
    phases.count = 4;
    setPreInfusionPhases(0, preinfuseBar, preinfuseTime, preinfuseSoak);
    setPhase(3, 9, 9, 1000);
    preInfusionFinishedPhaseIdx = 3;
    break;
  case 2: // No PI, yes PP
    phases.count = 2;
    setPresureProfilePhases(0, ppStartBar, ppFinishBar, ppHold, ppLength);
    preInfusionFinishedPhaseIdx = 0;
    break;
  case 4: // Both PI + PP
    phases.count = 5;
    setPreInfusionPhases(0, preinfuseBar, preinfuseTime, preinfuseSoak);
    setPresureProfilePhases(3, ppStartBar, ppFinishBar, ppHold, ppLength);
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

void newPressureProfile() {
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
  setPumpPressure(livePressure, newBarValue);
  // saving the target pressure
  pressureTargetComparator = preinfusionFinished ? (int) newBarValue : livePressure;
  // Keep that water at temp
  justDoCoffee();
}

void manualPressureProfile() {
  if( selectedOperationalMode == 3 ) {
    int power_reading = myNex.readNumber("h0.val");
    setPumpPressure(livePressure, power_reading);
  }
  justDoCoffee();
}

//#############################################################################################
//###############################____INIT_AND_ADMIN_CTRL____###################################
//#############################################################################################

void brewDetect() {
  if ( brewState() ) {
    openValve();
    /* Applying the below block only when brew detected */
    if (selectedOperationalMode == 0 || selectedOperationalMode == 1 || selectedOperationalMode == 2 || selectedOperationalMode == 3 || selectedOperationalMode == 4) {
      brewTimer(1); // nextion timer start
      brewActive = true;
      weighingStartRequested = true; // Flagging weighing start
      myNex.writeNum("warmupState", 0); // Flaggig warmup notification on Nextion needs to stop (if enabled)
      if (myNex.currentPageId == 1 || myNex.currentPageId == 2 || myNex.currentPageId == 8 || homeScreenScalesEnabled ) calculateWeight();
    } else if (selectedOperationalMode == 5 || selectedOperationalMode == 9) setPumpToRawValue(127); // setting the pump output target to 9 bars for non PP or PI profiles
    else if (selectedOperationalMode == 6) brewTimer(1); // starting the timerduring descaling
  } else{
    closeValve();
    brewTimer(0); // stopping timer
    brewActive = false;
    /* UPDATE VARIOUS INTRASHOT TIMERS and VARS */
    brewingTimer = millis();
    /* Only resetting the brew activity value if it's been previously set */
    preinfusionFinished = false;
    if (myNex.currentPageId == 1 || myNex.currentPageId == 2 || myNex.currentPageId == 8 || homeScreenScalesEnabled ) {
      /* Only setting the weight activity value if it's been previously unset */
      weighingStartRequested=true;
      calculateWeight();
    } else {/* Only resetting the scales values if on any other screens than brew or scales */
      weighingStartRequested = false; // Flagging weighing stop
      tareDone = false;
      previousBrewState = false;
      currentWeight = 0.f;
      previousWeight = 0.f;
    }
  }
}

void scalesInit() {

  #if defined(SINGLE_HX711_CLOCK)
    LoadCells.begin(HX711_dout_1, HX711_dout_2, HX711_sck_1);
    LoadCells.set_scale(scalesF1, scalesF2);
    LoadCells.power_up();

    delay(500);

    if (LoadCells.is_ready()) {
      LoadCells.tare(5);
      scalesPresent = true;
    }
  #else
    LoadCell_1.begin(HX711_dout_1, HX711_sck_1);
    LoadCell_2.begin(HX711_dout_2, HX711_sck_2);
    LoadCell_1.set_scale(scalesF1); // calibrated val1
    LoadCell_2.set_scale(scalesF2); // calibrated val2

    delay(500);

    if (LoadCell_1.is_ready() && LoadCell_2.is_ready()) {
      scalesPresent = true;
      LoadCell_1.tare();
      LoadCell_2.tare();
    }
  #endif
}

void scalesTare() {
  if( scalesPresent && (!tareDone || !previousBrewState) ) {
    #if defined(SINGLE_HX711_CLOCK)
      if (LoadCells.is_ready()) LoadCells.tare(5);
    #else
      if (LoadCell_1.wait_ready_timeout(300) && LoadCell_2.wait_ready_timeout(300)) {
        LoadCell_1.tare(2);
        LoadCell_2.tare(2);
      }
    #endif
    tareDone=1;
    previousBrewState=1;
  }
}

void lcdInit() {
  eepromValues_t eepromCurrentValues = eepromGetCurrentValues();

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

  myNex.writeNum("regHz", eepromCurrentValues.powerLineFrequency);

  myNex.writeNum("homeOnBrewFinish", eepromCurrentValues.homeOnShotFinish);
  myNex.writeNum("brewSettings.btGoHome.val", eepromCurrentValues.homeOnShotFinish);

  myNex.writeNum("graphEnabled", eepromCurrentValues.graphBrew);
  myNex.writeNum("brewSettings.btGraph.val", eepromCurrentValues.graphBrew);

  myNex.writeNum("warmupState", eepromCurrentValues.warmupState);
  myNex.writeNum("morePower.bt0.val", eepromCurrentValues.warmupState);

  myNex.writeNum("deltaState", eepromCurrentValues.brewDeltaState);
  myNex.writeNum("brewSettings.btTempDelta.val", eepromCurrentValues.brewDeltaState);
}

void dbgInit() {
  #if defined(STM32F4xx) && defined(DEBUG_ENABLED)
  analogReadResolution(12);
  #endif
}
void dbgOutput() {
  #if defined(STM32F4xx) && defined(DEBUG_ENABLED)
  int VRef = readVref();
  myNex.writeNum("debug1",readTempSensor(VRef));
  myNex.writeNum("debug2",getAdsError());
  #endif
}
