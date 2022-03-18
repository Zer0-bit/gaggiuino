#include <EEPROM.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>
#include <HX711.h>
#include <PSM.h>


#if defined(ARDUINO_ARCH_AVR)
  // ATMega32P pins definitions
  #define zcPin 2
  #define thermoDO 4
  #define thermoCS 5
  #define thermoCLK 6
  #define steamPin 7
  #define relayPin 8  // PB0
  #define dimmerPin 9
  #define brewPin A0 // PD7
  #define pressurePin A1 
  #define HX711_dout_1 12 //mcu > HX711 no 1 dout pin
  #define HX711_dout_2 13 //mcu > HX711 no 2 dout pin
  #define HX711_sck_1 10 //mcu > HX711 no 1 sck pin
  #define HX711_sck_2 11 //mcu > HX711 no 2 sck pin
  #define USART_CH Serial

  // configuration for TimerInterruptGeneric
  #define TIMER_INTERRUPT_DEBUG 0
  #define _TIMERINTERRUPT_LOGLEVEL_ 0

  #define USING_16MHZ true
  #define USING_8MHZ false
  #define USING_250KHZ false

  #define USE_TIMER_0 false
  #define USE_TIMER_1 true
  #define USE_TIMER_2 false
  #define USE_TIMER_3 false

  #include "TimerInterrupt_Generic.h"

#elif defined(ARDUINO_ARCH_STM32)// if arch is stm32
  #define SS 8
  // STM32F4 pins definitions
  #define zcPin PB0
  //#define thermoDO PB4
  #define thermoCS PB5
  //#define thermoCLK PB6
  #define brewPin PA0 // PD7
  #define relayPin PB8  // PB0
  #define dimmerPin PB9
  #define pressurePin PA1 
  #define steamPin PA2
  #define HX711_dout_1 PB14 //mcu > HX711 no 1 dout pin
  #define HX711_dout_2 PB15 //mcu > HX711 no 2 dout pin
  #define HX711_sck_1 PB12 //mcu > HX711 no 1 sck pin
  #define HX711_sck_2 PB13 //mcu > HX711 no 2 sck pin
  #define USART_CH Serial1
#endif


// Define some const values
#define GET_KTYPE_READ_EVERY 350 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#define GET_PRESSURE_READ_EVERY 50
#define GET_SCALES_READ_EVERY 200
#define REFRESH_SCREEN_EVERY 350 // Screen refresh interval (ms)
#define DESCALE_PHASE1_EVERY 500 // short pump pulses during descale
#define DESCALE_PHASE2_EVERY 5000 // short pause for pulse effficience activation
#define DESCALE_PHASE3_EVERY 120000 // long pause for scale softening
#define MAX_SETPOINT_VALUE 110 //Defines the max value of the setpoint
#define EEPROM_RESET 1 //change this value if want to reset to defaults
#define PUMP_RANGE 127

//########################__MAX_6675__####################################
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
//########################__NEXTION__#####################################
EasyNex myNex(USART_CH);
//#######################__Banoz_PSM__#################################### - for more cool shit visit https://github.com/banoz  and don't forget to star
PSM pump(zcPin, dimmerPin, PUMP_RANGE, FALLING);
//#######################__HX711_stuff__##################################
HX711 LoadCell_1; //HX711 1
HX711 LoadCell_2; //HX711 2

//########################__GLOBAL_VARIABLES__DECLARATION__#####################################
//volatile vars
volatile float kProbeReadValue; //temp val
volatile float livePressure;
volatile float liveWeight;
volatile unsigned int value; //dimmer value

// timers
volatile unsigned long thermoTimer; //temp timer
volatile unsigned long pressureTimer;
volatile unsigned long scalesTimer;
volatile unsigned long pageRefreshTimer;

volatile float newBarValue;
//scales vars
float scalesF1;
float scalesF2;
float currentWeight;
float previousWeight;
uint8_t targetWeight;



bool POWER_ON;
bool  descaleCheckBox;
bool  preinfusionState;
bool  pressureProfileState;
bool  warmupEnabled;
bool  flushEnabled;
bool  descaleEnabled;
bool preinfusionFinished;
bool scalesPresent;
volatile uint16_t  HPWR;
volatile float  HPWR_OUT;
uint16_t  setPoint;
uint16_t  offsetTemp;
uint8_t  MainCycleDivider;
uint8_t  BrewCycleDivider;
uint8_t  preinfuseTime;
uint8_t preinfuseBar;
uint8_t preinfuseSoak;
uint8_t ppStartBar;
uint8_t ppFinishBar;
uint8_t ppHold;
uint8_t ppLength;
uint8_t selectedOperationalMode;
uint8_t regionHz;

// EEPROM  stuff
const uint16_t  EEP_SETPOINT = 1;
const uint16_t  EEP_OFFSET = 20;
const uint16_t  EEP_HPWR = 40;
const uint16_t  EEP_M_DIVIDER = 60;
const uint16_t  EEP_B_DIVIDER = 80;
const uint16_t  EEP_P_START = 100;
const uint16_t  EEP_P_FINISH = 120;
const uint16_t  EEP_P_HOLD = 110;
const uint16_t  EEP_P_LENGTH = 130;
const uint16_t  EEP_PREINFUSION = 140;
const uint16_t  EEP_P_PROFILE = 160;
const uint16_t  EEP_PREINFUSION_SEC = 180;
const uint16_t  EEP_PREINFUSION_BAR = 190;
const uint16_t  EEP_PREINFUSION_SOAK = 170; 
const uint16_t  EEP_REGPWR_HZ = 195;
const uint16_t  EEP_WARMUP = 200;
const uint16_t  EEP_HOME_ON_SHOT_FINISH = 205;
const uint16_t  EEP_GRAPH_BREW = 210;
const uint16_t  EEP_SCALES_F1 = 215;
const uint16_t  EEP_SCALES_F2 = 220;
//########################__GLOBAL_VARIABLES__END__#####################################


//##############################################################################################################################
//############################################________________INIT______________################################################
//##############################################################################################################################
void setup() {
  
  USART_CH.begin(115200); // switching our board to the new serial speed

  // relay port init and set initial operating mode
  pinMode(relayPin, OUTPUT);
  pinMode(brewPin, INPUT_PULLUP);
  pinMode(steamPin, INPUT_PULLUP);

  // relayPin LOW
  setBoiler(LOW); 
  //Pump 
  pump.set(0);
  // Will wait hereuntil full serial is established, this is done so the LCD fully initializes before passing the EEPROM values
  while (myNex.readNumber("safetyTempCheck") != 100 )
  {
    delay(5);
  }
  
  // Initialising the vsaved values or writing defaults if first start
  eepromInit();
  // start interrupt read for pressure transducer
  initPressure(myNex.readNumber("regHz"));
  // Scales handling
  scalesInit();

  myNex.lastCurrentPageId = myNex.currentPageId;
  POWER_ON = true;
}

//##############################################################################################################################
//############################################________________MAIN______________################################################
//##############################################################################################################################


//Main loop where all the logic is continuously run
void loop() {
  pageValuesRefresh();
  myNex.NextionListen();
  sensorsRead();
  modeSelect();
  lcdRefresh();
}

//##############################################################################################################################
//#############################################___________SENSORS_READ________##################################################
//##############################################################################################################################


void sensorsRead() { // Reading the thermocouple temperature
  // Reading the temperature every 350ms between the loops
  if (millis() > thermoTimer) {
    kProbeReadValue = thermocouple.readCelsius();

    if (kProbeReadValue <= 0.0 || kProbeReadValue == NAN || kProbeReadValue > 165.0) { // safety measures
      setBoiler(LOW);  // boilerPin -> LOW
    }
    thermoTimer = millis() + GET_KTYPE_READ_EVERY;
  }

  livePressure = getPressure();


//  if (millis() > scalesTimer) {
//    float values[2];
//    scales.get_units(values);
//    liveWeight = values[0] + values[1];
//    scalesTimer = millis() + GET_SCALES_READ_EVERY;
//  }
}

//##############################################################################################################################
//############################################______PRESSURE_____TRANSDUCER_____################################################
//##############################################################################################################################
#if defined(ARDUINO_ARCH_AVR)
volatile unsigned int presData[2];
volatile unsigned char presIndex = 0;

void presISR()
{
  presData[presIndex] = ADCW;
  presIndex ^= 1;
}
#endif

void initPressure(uint8_t hz)
{
  #if defined(ARDUINO_ARCH_AVR)
    unsigned int pin = pressurePin - 14;
    ADMUX = (DEFAULT << 6) | (pin & 0x07);
    ADCSRB = (1 << ACME);
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    ITimer1.init();
    ITimer1.attachInterrupt(hz * 2, presISR);
  #endif 
}

float getPressure() {  //returns sensor pressure data
    // 5V/1024 = 1/204.8
    // voltageZero = 0.5V --> 102.4
    // voltageMax = 4.5V --> 921.6
    // range 921.6 - 102.4 = 819.2
    // pressure gauge range 0-1.2MPa - 0-12 bar
    // 1 bar = 68.27
    #if defined(ARDUINO_ARCH_AVR)
      return (presData[0] + presData[1]) / 136.54 - 1.49f;
    #else
      return analogRead(pressurePin) / 68.27 - 1.49f;
    #endif
}


void setPressure(int targetValue) {  
 if (targetValue == 0 || livePressure > targetValue) {
   pump.set(0);
 } else {
	unsigned int pumpValue = 127 - livePressure * 12;
	if (livePressure > targetValue) pumpValue = 0;
	pump.set(pumpValue);
 }
}

//##############################################################################################################################
//############################################______PAGE_CHANGE_VALUES_REFRESH_____#############################################
//##############################################################################################################################

void pageValuesRefresh() {  // Refreshing our values on page changes

  if ( myNex.currentPageId != myNex.lastCurrentPageId || POWER_ON == true ) {
    preinfusionState = myNex.readNumber("piState"); // reding the preinfusion state value which should be 0 or 1
    pressureProfileState = myNex.readNumber("ppState"); // reding the pressure profile state value which should be 0 or 1
    preinfuseTime = myNex.readNumber("piSec");
    preinfuseBar = myNex.readNumber("piBar"); 
    preinfuseSoak = myNex.readNumber("piSoak"); // pre-infusion soak value
    ppStartBar = myNex.readNumber("ppStart");
    ppFinishBar = myNex.readNumber("ppFin");
    ppHold = myNex.readNumber("ppHold"); // pp start pressure hold
    ppLength = myNex.readNumber("ppLength"); // pp shot length
    flushEnabled = myNex.readNumber("flushState");
    descaleEnabled = myNex.readNumber("descaleState");
    setPoint = myNex.readNumber("setPoint");  // reading the setPoint value from the lcd
    offsetTemp = myNex.readNumber("offSet");  // reading the offset value from the lcd
    HPWR = myNex.readNumber("hpwr");  // reading the brew time delay used to apply heating in waves
    MainCycleDivider = myNex.readNumber("mDiv");  // reading the delay divider
    BrewCycleDivider = myNex.readNumber("bDiv");  // reading the delay divider
    regionHz = myNex.readNumber("regHz");
    warmupEnabled = myNex.readNumber("warmupState");

    // MODE_SELECT should always be last
    selectedOperationalMode = myNex.readNumber("modeSelect");

    myNex.lastCurrentPageId = myNex.currentPageId;
    POWER_ON = false;
  }
}

//#############################################################################################
//############################____OPERATIONAL_MODE_CONTROL____#################################
//#############################################################################################
void modeSelect() {
  switch (selectedOperationalMode) {
    case 0: //state = STRAIGHT 9 bar
      if (steamState() == 0) justDoCoffee();
      else steamCtrl();
      break;
    case 1: //state = PREINFUSION
      if (steamState() == 0) preInfusion();
      else steamCtrl();
      break;
    case 2: //state = PRESSURE PROFILING
      if (steamState() == 0) autoPressureProfile();
      else steamCtrl();
      break;
    case 3: //state = MANUAL PP
      manualPressureProfile();
      break;
    case 4: //state = PP (with or without PI)
      if (steamState() == 0) {
        if(preinfusionFinished == false) preInfusion();
        else if(preinfusionFinished == true) autoPressureProfile();
      } else if (steamState() == 1) steamCtrl();
      break;
    case 5: //state = FLUSH
      if (steamState() == 0) justDoCoffee();
      else steamCtrl();
      break;
    case 6: //state = DESCALE
      deScale(descaleCheckBox);
      break;
    case 7: //state = N/A
      break;
    case 8: //state = N/A
      break;
    case 9: //state = STEAMING
      if (steamState() == 0) justDoCoffee();
      else steamCtrl();
      break;
    default: //state = default state in case of logical omission
      if (steamState() == 0) justDoCoffee();
      else steamCtrl();
      break;
  }
}

//#############################################################################################
//#########################____NO_OPTIONS_ENABLED_POWER_CONTROL____############################
//#############################################################################################
void justDoCoffee() {
  uint8_t HPWR_LOW = HPWR/MainCycleDivider;
  static double heaterWave;
  static uint8_t heaterState, heaterTempDirection;
  float BREW_TEMP_DELTA;
  // Calculating the boiler heating power range based on the below input values
  HPWR_OUT = mapRange(kProbeReadValue, setPoint - 10, setPoint, HPWR, HPWR_LOW, 0);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, HPWR);  // limits range of sensor values to HPWR_LOW and HPWR
  BREW_TEMP_DELTA = mapRange(kProbeReadValue, setPoint, setPoint+setPoint*0.10, setPoint*0.10, 0, 0);
  BREW_TEMP_DELTA = constrain(BREW_TEMP_DELTA, 0,  setPoint*0.10);


  if (brewState() == 1) {
    if (selectedOperationalMode == 0) {
      setPressure(9);
      brewTimer(1);
    }else if (selectedOperationalMode == 5 || selectedOperationalMode == 9) setPressure(9);
    myNex.writeNum("warmupState", 0);
  // Applying the HPWR_OUT variable as part of the relay switching logic
    if (kProbeReadValue > setPoint-1.5f && kProbeReadValue < setPoint+0.25 && !preinfusionFinished ) {
      if (millis() - heaterWave > HPWR_LOW*BrewCycleDivider && !heaterState ) {
        setBoiler(LOW);  // relayPin -> LOW
        heaterState=1;
        heaterWave=millis();
      }else if (millis() - heaterWave > HPWR && heaterState ) {
        setBoiler(HIGH);  // relayPin -> HIGH
        heaterState=0;
        heaterWave=millis();
      }
    } else if (kProbeReadValue > setPoint-1.5f && kProbeReadValue < setPoint+BREW_TEMP_DELTA && preinfusionFinished ) {
    if (millis() - heaterWave > HPWR*BrewCycleDivider && !heaterState ) {
      setBoiler(HIGH);  // relayPin -> HIGH
      heaterState=1;
      heaterWave=millis();
    }else if (millis() - heaterWave > HPWR && heaterState ) {
      setBoiler(LOW);  // relayPin -> LOW
      heaterState=0;
      heaterWave=millis();
    }
  } else if(kProbeReadValue <= setPoint-1.5f) setBoiler(HIGH);   // relayPin -> HIGH
    else {
      setBoiler(LOW);  // relayPin -> LOW
    }
  } else { //if brewState == 0
    brewTimer(0);
    if (kProbeReadValue < ((float)setPoint - 10.00)) {
      setBoiler(HIGH);  // relayPin -> HIGH
    } else if (kProbeReadValue >= ((float)setPoint - 10.00) && kProbeReadValue < ((float)setPoint - 3.00)) {
      setBoiler(HIGH);  // relayPin -> HIGH
      if (millis() - heaterWave > HPWR_OUT/BrewCycleDivider) {
        setBoiler(LOW);  // relayPin -> LOW
        heaterState=0;
        heaterWave=millis();
      }
    } else if ((kProbeReadValue >= ((float)setPoint - 3.00)) && (kProbeReadValue <= ((float)setPoint - 1.00))) {
      if (millis() - heaterWave > HPWR_OUT/BrewCycleDivider && heaterState == 0) {
        setBoiler(HIGH);  // relayPin -> HIGH
        heaterState=1;
        heaterWave=millis();
      }else if (millis() - heaterWave > HPWR_OUT/BrewCycleDivider && heaterState == 1) {
        setBoiler(LOW);  // relayPin -> LOW
        heaterState=0;
        heaterWave=millis();
      } 
    } else if ((kProbeReadValue >= ((float)setPoint - 0.5)) && kProbeReadValue < (float)setPoint) {
      if (millis() - heaterWave > HPWR_OUT/BrewCycleDivider && heaterState == 0) {
        setBoiler(HIGH);  // relayPin -> HIGH
        heaterState=1;
        heaterWave=millis();
      }else if (millis() - heaterWave > HPWR_OUT/BrewCycleDivider && heaterState == 1) {
        setBoiler(LOW);  // relayPin -> LOW
        heaterState=0;
        heaterWave=millis();
      }
    } else {
      setBoiler(LOW);  // relayPin -> LOW
    }
  }
}

//#############################################################################################
//################################____STEAM_POWER_CONTROL____##################################
//#############################################################################################

void steamCtrl() {
  float boilerPressure = getPressure();

  if (brewState() == 0) {
    if (boilerPressure <= 9.0) { // steam temp control, needs to be aggressive to keep steam pressure acceptable
      if ((kProbeReadValue > setPoint-10.00) && (kProbeReadValue <=155)) setBoiler(HIGH);  // relayPin -> HIGH
      else setBoiler(LOW);  // relayPin -> LOW
    }else if(boilerPressure >=9.1) setBoiler(LOW);  // relayPin -> LOW
  }else if (brewState() == 1) { //added to cater for hot water from steam wand functionality
  if (boilerPressure <= 9.0) {
      if ((kProbeReadValue > setPoint-10.00) && (kProbeReadValue <=105)) setBoiler(HIGH);  // relayPin -> HIGH
      else setBoiler(LOW);  // relayPin -> LOW
    }else if(boilerPressure >=9.1) setBoiler(LOW);  // relayPin -> LOW
  }else setBoiler(LOW);  // relayPin -> LOW
}

//#############################################################################################
//################################____LCD_REFRESH_CONTROL___###################################
//#############################################################################################

void lcdRefresh() {
  // Updating the LCD every 300ms
  static unsigned long pageRefreshTimer, scalesRefreshTimer, refreshTimer;
  static bool tareDone, previousBrewState;
  static uint8_t wErr;
  static float fWghtEntryVal;
  float flowVal;
  //float gPressureCurveBeautify = newBarValue*10;
  
  if (millis() - pageRefreshTimer > REFRESH_SCREEN_EVERY) {
	//(preinfusionFinished == true) ? myNex.writeNum("pressure.val", int(gPressureCurveBeautify)) : myNex.writeNum("pressure.val", int(livePressure*10));
	myNex.writeNum("pressure.val", int(livePressure*10));
    myNex.writeNum("currentTemp",int(kProbeReadValue-offsetTemp));
    pageRefreshTimer = millis();
  }
  if (brewState() == 1 && scalesPresent == true ) {
    if (myNex.currentPageId == 1 || myNex.currentPageId == 2 || myNex.currentPageId == 8) {
      TARE_AGAIN:
      if(tareDone != 1 || previousBrewState != 1) {
        if (LoadCell_1.wait_ready_timeout(150) && LoadCell_2.wait_ready_timeout(150)) {
          LoadCell_1.tare();
          LoadCell_2.tare();
        }
        tareDone=1;
        previousBrewState=1;
      }
      if (millis() - scalesRefreshTimer > 200) {
        currentWeight = (LoadCell_1.get_units() + LoadCell_2.get_units()) / 2;
        if (currentWeight <= -0.5 || (currentWeight-previousWeight) >= 20.0) {
          tareDone=0;
          previousBrewState=0;
          goto TARE_AGAIN;
        }
        // soft smooth quite dumb atm just wanted ot have a more stable output value
        if (currentWeight > 1.5 && currentWeight<previousWeight && wErr < 8) {
          currentWeight = previousWeight; 
          wErr++;
        }else if (currentWeight > 1.5 && currentWeight<previousWeight && wErr >= 8) {
          previousWeight = currentWeight;
          wErr = 0;
        }else previousWeight = currentWeight;// smoothing end
        scalesRefreshTimer = millis();
      } 
      myNex.writeStr("weight.txt",String(currentWeight,1));
      // FLow calc
      if ((currentWeight - fWghtEntryVal) >= 0.5) {
        if (millis() - refreshTimer >= 1000) {
          flowVal = (currentWeight - fWghtEntryVal)*10;
          myNex.writeNum("flow.val", int(flowVal));
          fWghtEntryVal = currentWeight;
          refreshTimer = millis();
        }
      }
    }
  }else if (brewState() == 0 && scalesPresent == true  && (myNex.currentPageId == 1 || myNex.currentPageId == 2||myNex.currentPageId == 8)) {
    myNex.writeStr("weight.txt",String(currentWeight+flowVal,1));
    previousBrewState=0;
    tareDone=0;
  }
  else if (brewState() == 0 && scalesPresent == true && myNex.currentPageId == 11) {//scales screen updating
    if (millis() - scalesRefreshTimer > 200) {
      if(tareDone != 1) {
        if (LoadCell_1.wait_ready_timeout(100) && LoadCell_2.wait_ready_timeout(100)) {
          LoadCell_1.tare();
          LoadCell_2.tare();
        }
        tareDone=1;
      }
      currentWeight = (LoadCell_1.get_units() + LoadCell_2.get_units()) / 2;
      myNex.writeStr("weight.txt",String(currentWeight,1));
      // soft smooth quite dumb atm just wanted ot have a more stable output value
      if (currentWeight > 1.5 && currentWeight<previousWeight && wErr < 4) {
        currentWeight = previousWeight; 
        wErr++;
      }else if (currentWeight > 1.5 && currentWeight<previousWeight && wErr >= 4) {
        previousWeight = currentWeight;
        wErr = 0;
      }// smoothing end
      previousBrewState=0;
      scalesRefreshTimer = millis();
    }
  }else {
    previousBrewState=0;
    tareDone=0;
    currentWeight=0;
    previousWeight=0;
    fWghtEntryVal=0;
    wErr=0;
  }
}
//#############################################################################################
//###################################____SAVE_BUTTON____#######################################
//#############################################################################################
// Save the desired temp values to EEPROM
void trigger1() {
  uint16_t valueToSave; 
  uint8_t allValuesUpdated;

  switch (myNex.currentPageId){
    case 1:
      break;
    case 2:
      break;
    case 3:
      // Saving ppStart,ppFin,ppHold and ppLength
      valueToSave = myNex.readNumber("ppStart");
      if (valueToSave != 0 && valueToSave >= 1) {
        EEPROM.put(EEP_P_START, valueToSave);
        allValuesUpdated++;
      }else {}
      valueToSave = myNex.readNumber("ppFin");
      if (valueToSave != 0 && valueToSave >= 1) {
        EEPROM.put(EEP_P_FINISH, valueToSave);
        allValuesUpdated++;
      }else {}
      valueToSave = myNex.readNumber("ppHold");
      if (valueToSave >= 0) {
        EEPROM.put(EEP_P_HOLD, valueToSave);
        allValuesUpdated++;
      }else {}
      valueToSave = myNex.readNumber("ppLength");
      if (valueToSave >= 0) {
        EEPROM.put(EEP_P_LENGTH, valueToSave);
        allValuesUpdated++;
      }else {}
      // Saving PI and PP
      valueToSave = myNex.readNumber("piState");
      if (valueToSave == 0 || valueToSave == 1 ) {
        EEPROM.put(EEP_PREINFUSION, valueToSave);
        allValuesUpdated++;
      }else {}
      valueToSave = myNex.readNumber("ppState");
      if (valueToSave == 0 || valueToSave == 1 ) {
        EEPROM.put(EEP_P_PROFILE, valueToSave);
        allValuesUpdated++;
      }else {}
      //Saved piSec
      valueToSave = myNex.readNumber("piSec");
      if ( valueToSave >= 0 ) {
        EEPROM.put(EEP_PREINFUSION_SEC, valueToSave);
        allValuesUpdated++;
      }else {}
      //Saved piBar
      valueToSave = myNex.readNumber("piBar");
      if ( valueToSave >= 0 && valueToSave <= 9) {
        EEPROM.put(EEP_PREINFUSION_BAR, valueToSave);
        allValuesUpdated++;
      }else {}
      //Saved piSoak
      valueToSave = myNex.readNumber("piSoak");
      if ( valueToSave >= 0 ) {
        EEPROM.put(EEP_PREINFUSION_SOAK, valueToSave);
        allValuesUpdated++;
      }else {}
      if (allValuesUpdated == 9) {
        allValuesUpdated=0;
        myNex.writeStr("popupMSG.t0.txt","UPDATE SUCCESSFUL!");
      }else myNex.writeStr("popupMSG.t0.txt","ERROR!");
      myNex.writeStr("page popupMSG");
      break;
    case 4:
      //Saving brewSettings
      valueToSave = myNex.readNumber("homeOnBrewFinish");
      if ( valueToSave >= 0 ) {
        EEPROM.put(EEP_HOME_ON_SHOT_FINISH, valueToSave);
        allValuesUpdated++;
      }else {}
      valueToSave = myNex.readNumber("graphEnabled");
      if ( valueToSave >= 0 ) {
        EEPROM.put(EEP_GRAPH_BREW, valueToSave);
        allValuesUpdated++;
      }else {}
      if (allValuesUpdated == 2) {
        allValuesUpdated=0;
        myNex.writeStr("popupMSG.t0.txt","UPDATE SUCCESSFUL!");
      }else myNex.writeStr("popupMSG.t0.txt","ERROR!");
      myNex.writeStr("page popupMSG");
      break;
    case 5:
      break;
    case 6: 
      // Reading the LCD side set values
      valueToSave = myNex.readNumber("setPoint");
      if ( valueToSave > 0) { 
        EEPROM.put(EEP_SETPOINT, valueToSave);
        allValuesUpdated++;
      }else {}
      // Saving offset
      valueToSave = myNex.readNumber("offSet");
      if ( valueToSave >= 0 ) {
        EEPROM.put(EEP_OFFSET, valueToSave);
        allValuesUpdated++;
      }else {}
      // Saving HPWR
      valueToSave = myNex.readNumber("hpwr");
      if ( valueToSave >= 0 ) {
        EEPROM.put(EEP_HPWR, valueToSave);
        allValuesUpdated++;
      }else {}
      // Saving mDiv
      valueToSave = myNex.readNumber("mDiv");
      if ( valueToSave >= 1) {
        EEPROM.put(EEP_M_DIVIDER, valueToSave);
        allValuesUpdated++;
      }else {}
      //Saving bDiv
      valueToSave = myNex.readNumber("bDiv");
      if ( valueToSave >= 1) {
        EEPROM.put(EEP_B_DIVIDER, valueToSave);
        allValuesUpdated++;
      }else {}
      if (allValuesUpdated == 5) {
        allValuesUpdated=0;
        myNex.writeStr("popupMSG.t0.txt","UPDATE SUCCESSFUL!");
      }else myNex.writeStr("popupMSG.t0.txt","ERROR!");
      myNex.writeStr("page popupMSG");
      break;
    case 7:
      valueToSave = myNex.readNumber("regHz");
      if ( valueToSave == 50 || valueToSave == 60 ) {
        EEPROM.put(EEP_REGPWR_HZ, valueToSave);
#if defined(ARDUINO_ARCH_AVR)
        ITimer1.attachInterrupt(valueToSave * 2, presISR);
#endif
        allValuesUpdated++;
      }else {}
      // Saving warmup state
      valueToSave = myNex.readNumber("warmupState");
      if (valueToSave == 0 || valueToSave == 1 ) {
        EEPROM.put(EEP_WARMUP, valueToSave);
        allValuesUpdated++;
      }else {}
      if (allValuesUpdated == 2) {
        allValuesUpdated=0;
        myNex.writeStr("popupMSG.t0.txt","UPDATE SUCCESSFUL!");
      }else myNex.writeStr("popupMSG.t0.txt","ERROR!");
      myNex.writeStr("page popupMSG");
      break;
    default:
      break;
  }
}

//#############################################################################################
//###################################_____SCALES_TARE____######################################
//#############################################################################################

void trigger2() {
  if (LoadCell_1.wait_ready_timeout(100) && LoadCell_2.wait_ready_timeout(100)) {
    LoadCell_1.tare();
    LoadCell_2.tare();
  }
}

//#############################################################################################
//###############################_____HELPER_FUCTIONS____######################################
//#############################################################################################

//Function to get the state of the brew switch button
//returns true or false based on the read P(power) value
bool brewState() {  //Monitors the current flowing through the ACS712 circuit and returns a value depending on the power value (P) the system draws
 return (digitalRead(brewPin) != LOW ) ? 0 : 1; // pin will be high when switch is ON.
}

// Returns HIGH when switch is OFF and LOW when ON
// pin will be high when switch is ON.
bool steamState() {
  return (digitalRead(steamPin) != LOW) ? 0 : 1;
}

bool brewTimer(bool c) { // small function for easier timer start/stop
  if ( c == 1) myNex.writeNum("timerState", 1);
  else myNex.writeNum("timerState", 0);
}

// Actuating the heater element
void setBoiler(uint8_t val) {
  #if defined(ARDUINO_ARCH_AVR)
    if (val == HIGH) {
      PORTB |= _BV(PB0);  // boilerPin -> HIGH
    } else {
      PORTB &= ~_BV(PB0);  // boilerPin -> LOW
    }
  #elif defined(ARDUINO_ARCH_STM32)// if arch is stm32
    if (val == HIGH) {
      digitalWrite(relayPin, HIGH);  // boilerPin -> HIGH
    } else {
      digitalWrite(relayPin, LOW);   // boilerPin -> LOW
    }
  #endif
}

float mapRange(float sourceNumber, float fromA, float fromB, float toA, float toB, int decimalPrecision ) {
  float deltaA = fromB - fromA;
  float deltaB = toB - toA;
  float scale  = deltaB / deltaA;
  float negA   = -1 * fromA;
  float offset = (negA * scale) + toA;
  float finalNumber = (sourceNumber * scale) + offset;
  int calcScale = (int) pow(10, decimalPrecision);
  return (float) round(finalNumber * calcScale) / calcScale;
}


float smoothValue(float inputVal) {
  // Define the number of samples to keep track of. The higher the number, the
  // more the readings will be smoothed, but the slower the output will respond to
  // the input. Using a constant rather than a normal variable lets us use this
  // value to determine the size of the readings array.
  const int numReadings = 5;

  int readings[numReadings];      // the readings from the analog input
  int readIndex = 0;              // the index of the current reading
  int total = 0;                  // the running total
  int average = 0;                // the average

  // initialize serial communication with computer:
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = inputVal;
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  return average = total / numReadings;
}


//#############################################################################################
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

void deScale(bool c) {
  if (myNex.currentPageId==5) {
    static bool blink = true;
    static unsigned long timer = millis();
    static uint8_t currentCycleRead = myNex.readNumber("j0.val");
    static uint8_t lastCycleRead = 10;
    static bool descaleFinished = false;
    if (brewState() == 1 && descaleFinished == false) {
      brewTimer(1);
      if (currentCycleRead < lastCycleRead) { // descale in cycles for 5 times then wait according to the below condition
        if (blink == true) { // Logic that switches between modes depending on the $blink value
          setPressure(2);
          if (millis() - timer > DESCALE_PHASE1_EVERY) { //set dimmer power to max descale value for 10 sec
            if (currentCycleRead >=100) descaleFinished = true;
            blink = false;
            currentCycleRead = myNex.readNumber("j0.val");
            timer = millis();
          }
        }else {
          setPressure(1);
          if (millis() - timer > DESCALE_PHASE2_EVERY) { //set dimmer power to min descale value for 20 sec
            blink = true;
            currentCycleRead++;
            if (currentCycleRead<100) myNex.writeNum("j0.val", currentCycleRead);
            timer = millis();
          }
        }
      }else {
        setPressure(0);
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
    }else if (brewState() == 1 && descaleFinished == true){
      setPressure(0);
      if ((millis() - timer) > 1000) {
        brewTimer(0);
        myNex.writeStr("t14.txt", "FINISHED!");
        timer=millis();
      }
    }else if (brewState() == 0) {
      currentCycleRead = 0;
      lastCycleRead = 10;
      descaleFinished = false;
      timer = millis();
    }
   //keeping it at temp
    justDoCoffee();
  }
}


//#############################################################################################
//###############################____PRESSURE_CONTROL____######################################
//#############################################################################################


// Pressure profiling function, uses dimmer to dim the pump 
// Linear dimming as time passes, goes from pressure start to end incrementally or decrementally
void autoPressureProfile() {
  static bool phase_1 = 1, phase_2 = 0, updateTimer = 1;
  static unsigned long timer;
  //static float newBarValue;

  if (brewState() == 1) { //runs this only when brew button activated and pressure profile selected  
    if (updateTimer == 1) {
      timer = millis();
      updateTimer = 0;
    }
    if (phase_1 == true) { //enters phase 1
      if ((millis() - timer) > (ppHold*1000)) { // the actions of this if block are run after 4 seconds have passed since starting brewing
        phase_1 = 0;
        phase_2 = 1;
        timer = millis();
      }
      brewTimer(1);
      newBarValue=ppStartBar;
      setPressure(newBarValue);
    } else if (phase_2 == true) { //enters pahse 2
      if (ppStartBar < ppFinishBar) { // Incremental profiling curve
        newBarValue = mapRange(millis(),timer,timer + (ppLength*1000),ppStartBar,ppFinishBar,1); //Used to calculate the pressure drop/raise during a @ppLength sec shot
        if (newBarValue < (float)ppStartBar) newBarValue = (float)ppStartBar;
        else if (newBarValue > (float)ppFinishBar) newBarValue = (float)ppFinishBar;
      }else if (ppStartBar > ppFinishBar) { // Decremental profiling curve
        newBarValue = mapRange(millis(),timer,timer + (ppLength*1000),ppStartBar,ppFinishBar,1); //Used to calculate the pressure drop/raise during a @ppLength sec shot
        if (newBarValue > (float)ppStartBar) newBarValue = (float)ppStartBar;
        else if (newBarValue < ppFinishBar) newBarValue = (float)ppFinishBar;      
      }else { // Flat line profiling
        newBarValue = mapRange(millis(),timer,timer + (ppLength*1000),ppStartBar,ppFinishBar,1); //Used to calculate the pressure drop/raise during a @ppLength sec shot
        if (newBarValue < (float)ppStartBar) newBarValue = (float)ppStartBar;
        else if (newBarValue > (float)ppFinishBar) newBarValue = (float)ppFinishBar;
      }
      setPressure(newBarValue);
    }
  }else { 
    brewTimer(0);
    if (selectedOperationalMode == 1 ) setPressure(ppStartBar);
    else if (selectedOperationalMode == 4 ) preinfusionFinished = false;
    timer = millis();
    phase_2 = false;
    phase_1=true;
    updateTimer = 1;
    newBarValue = 0.0;
  }
 // Keep that water at temp
  justDoCoffee();
}

void manualPressureProfile() {
  if( myNex.currentPageId == 2 ) {
    uint8_t power_reading = myNex.readNumber("h0.val");
    if (brewState() == 1) {
      brewTimer(1);
      setPressure(power_reading);
    }else if (brewState() == 0) {
      brewTimer(0);
    }
  }

  justDoCoffee();
}
//#############################################################################################
//###############################____PREINFUSION_CONTROL____###################################
//#############################################################################################

// Pump dimming during brew for preinfusion
void preInfusion() {
  static bool blink = true;
  static bool exitPreinfusion;
  static unsigned long timer = millis();

  if ( brewState() == 1 ) {
    if (exitPreinfusion == false) { //main preinfusion body
      if (blink == true) { // Logic that switches between modes depending on the $blink value
        brewTimer(1);
        setPressure(preinfuseBar);
        if ((millis() - timer) >= (preinfuseTime*1000)) {
          blink = false;
          timer = millis();
        }
      }else {
        setPressure(0);
        if ((millis() - timer) >= (preinfuseSoak*1000)) { 
          exitPreinfusion = true;
          blink = true;
          timer = millis();
        }
      }
      // myNex.writeStr("t11.txt",String(getPressure(),1));
    }else if(exitPreinfusion == true && selectedOperationalMode == 1){ // PI
      setPressure(9);
    }else if(exitPreinfusion == true && selectedOperationalMode == 4){ // PI + PP
      preinfusionFinished = true;
      setPressure(ppStartBar);
    }
  }else if ( brewState() == 0 ) { //resetting all the values
    brewTimer(0);
    setPressure(preinfuseBar);
    exitPreinfusion = false;
    timer = millis();
  }
 //keeping it at temp
  justDoCoffee();
}

//#############################################################################################
//###############################____INIT_AND_ADMIN_CTRL____###################################
//#############################################################################################
void scalesInit() {
  LoadCell_1.begin(HX711_dout_1, HX711_sck_1);
  LoadCell_2.begin(HX711_dout_2, HX711_sck_2);
  
  if (LoadCell_1.is_ready() && LoadCell_2.is_ready()) {
    //good values - 1: 1708 2: -2162 | 
    LoadCell_1.set_scale(scalesF1); // calibrated val1
    LoadCell_2.set_scale(scalesF2); // calibrated val2
    scalesPresent = true;
  }
}


void eepromInit() {
	//If it's the first boot we'll need to set some defaults
  if (EEPROM.read(0) != EEPROM_RESET || EEPROM.read(EEP_SETPOINT) == 0 || EEPROM.read(EEP_SETPOINT) == 65535|| EEPROM.read(EEP_PREINFUSION_SOAK) == 65535) {
    USART_CH.println("SECU_CHECK FAILED! Applying defaults!");
    EEPROM.put(0, EEPROM_RESET);
    //The values can be modified to accomodate whatever system it tagets
    //So on first boot it writes and reads the desired system values
    EEPROM.put(EEP_SETPOINT, 100);
    EEPROM.put(EEP_OFFSET, 7);
    EEPROM.put(EEP_HPWR, 550);
    EEPROM.put(EEP_M_DIVIDER, 5);
    EEPROM.put(EEP_B_DIVIDER, 2);
    EEPROM.put(EEP_PREINFUSION, 0);
    EEPROM.put(EEP_P_START, 9);
    EEPROM.put(EEP_P_FINISH, 6);
    EEPROM.put(EEP_P_PROFILE, 0);
    EEPROM.put(EEP_PREINFUSION_SEC, 8);
    EEPROM.put(EEP_PREINFUSION_BAR, 2);
    EEPROM.put(EEP_REGPWR_HZ, 60);
    EEPROM.put(EEP_WARMUP, 0);
    EEPROM.put(EEP_GRAPH_BREW, 0);
    EEPROM.put(EEP_HOME_ON_SHOT_FINISH, 1);
    EEPROM.put(EEP_PREINFUSION_SOAK, 5);
    EEPROM.put(EEP_P_HOLD, 7);
    EEPROM.put(EEP_P_LENGTH, 30);
    EEPROM.put(EEP_GRAPH_BREW, 0);
	EEPROM.put(EEP_SCALES_F1, 1955.571428f);
	EEPROM.put(EEP_SCALES_F2, -2091.571428f);
  }

  // Applying our saved EEPROM saved values
  uint16_t init_val;
  // Loading the saved values fro EEPROM and sending them to the LCD

  EEPROM.get(EEP_SETPOINT, init_val);// reading setpoint value from eeprom
  if ( init_val > 0 ) {
    myNex.writeNum("setPoint", init_val);
    myNex.writeNum("moreTemp.n1.val", init_val);
  }
  EEPROM.get(EEP_OFFSET, init_val); // reading offset value from eeprom
  if ( init_val >= 0 ) {
    myNex.writeNum("offSet", init_val);
    myNex.writeNum("moreTemp.n2.val", init_val);
  }
  EEPROM.get(EEP_HPWR, init_val);//reading HPWR value from eeprom
  if (  init_val >= 0 ) {
    myNex.writeNum("hpwr", init_val);
    myNex.writeNum("moreTemp.n3.val", init_val);
  }
  EEPROM.get(EEP_M_DIVIDER, init_val);//reading main cycle div from eeprom
  if ( init_val >= 1 ) {
    myNex.writeNum("mDiv", init_val);
    myNex.writeNum("moreTemp.n4.val", init_val);
  }
  EEPROM.get(EEP_B_DIVIDER, init_val);//reading brew cycle div from eeprom
  if (  init_val >= 1 ) {
    myNex.writeNum("bDiv", init_val);
    myNex.writeNum("moreTemp.n5.val", init_val);
  }
  EEPROM.get(EEP_P_START, init_val);//reading pressure profile start value from eeprom
  if (  init_val >= 0 ) {
    myNex.writeNum("ppStart", init_val);
    myNex.writeNum("brewAuto.n2.val", init_val);
  }

  EEPROM.get(EEP_P_FINISH, init_val);// reading pressure profile finish value from eeprom
  if (  init_val >= 0 ) {
    myNex.writeNum("ppFin", init_val);
    myNex.writeNum("brewAuto.n3.val", init_val);
  }
  EEPROM.get(EEP_P_HOLD, init_val);// reading pressure profile hold value from eeprom
  if (  init_val >= 0 ) {
    myNex.writeNum("ppHold", init_val);
    myNex.writeNum("brewAuto.n5.val", init_val);
  }
  EEPROM.get(EEP_P_LENGTH, init_val);// reading pressure profile length value from eeprom
  if (  init_val >= 0 ) {
    myNex.writeNum("ppLength", init_val);
    myNex.writeNum("brewAuto.n6.val", init_val);
  }

  EEPROM.get(EEP_PREINFUSION, init_val);//reading preinfusion checkbox value from eeprom
  if (  !(init_val < 0) && init_val < 2 ) {
    myNex.writeNum("piState", init_val);
    myNex.writeNum("brewAuto.bt0.val", init_val);
  }

  EEPROM.get(EEP_P_PROFILE, init_val);//reading pressure profile checkbox value from eeprom
  if (  !(init_val < 0) && init_val < 2 ) {
    myNex.writeNum("ppState", init_val);
    myNex.writeNum("brewAuto.bt1.val", init_val);
  }

  EEPROM.get(EEP_PREINFUSION_SEC, init_val);//reading preinfusion time value from eeprom
  if (init_val >= 0) {
    myNex.writeNum("piSec", init_val);
    myNex.writeNum("brewAuto.n0.val", init_val);
  }

  EEPROM.get(EEP_PREINFUSION_BAR, init_val);//reading preinfusion pressure value from eeprom
  if (  init_val >= 0 && init_val < 9 ) {
    myNex.writeNum("piBar", init_val);
    myNex.writeNum("brewAuto.n1.val", init_val);
  }
  EEPROM.get(EEP_PREINFUSION_SOAK, init_val);//reading preinfusion soak times value from eeprom
  if (  init_val >= 0 ) {
    myNex.writeNum("piSoak", init_val);
    myNex.writeNum("brewAuto.n4.val", init_val);
  }
  // Region POWER value
  EEPROM.get(EEP_REGPWR_HZ, init_val);//reading region frequency value from eeprom
  if (  init_val == 50 || init_val == 60 ) myNex.writeNum("regHz", init_val);


  // Brew page settings
  EEPROM.get(EEP_HOME_ON_SHOT_FINISH, init_val);//reading bre time value from eeprom
  if (  init_val == 0 || init_val == 1 ) {
    myNex.writeNum("homeOnBrewFinish", init_val);
    myNex.writeNum("brewSettings.btGoHome.val", init_val);
  }

  EEPROM.get(EEP_GRAPH_BREW, init_val);//reading preinfusion pressure value from eeprom
  if (  init_val == 0 || init_val == 1) {
    myNex.writeNum("graphEnabled", init_val);
    myNex.writeNum("brewSettings.btGraph.val", init_val);
  }

  // Warmup checkbox value
  EEPROM.get(EEP_WARMUP, init_val);//reading preinfusion pressure value from eeprom
  if (  init_val == 0 || init_val == 1 ) {
    myNex.writeNum("warmupState", init_val);
    myNex.writeNum("morePower.bt0.val", init_val);
  }
  // Scales values
  EEPROM.get(EEP_SCALES_F1, scalesF1);//reading scale factors value from eeprom
  EEPROM.get(EEP_SCALES_F2, scalesF2);//reading scale factors value from eeprom
}
