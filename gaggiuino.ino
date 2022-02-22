#include <RBDdimmer.h>
#include <EEPROM.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>

// Define our pins
#define thermoDO 4
#define thermoCS 5
#define thermoCLK 6
#define steamPin 7
#define relayPin 8  // PB0
#define dimmerPin 9
#define brewPin A0 // PD7
#define pressurePin A1 

// Define some const values
#define GET_KTYPE_READ_EVERY 350 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#define REFRESH_SCREEN_EVERY 350 // Screen refresh interval (ms)
#define DESCALE_PHASE1_EVERY 500 // short pump pulses during descale
#define DESCALE_PHASE2_EVERY 5000 // short pause for pulse effficience activation
#define DESCALE_PHASE3_EVERY 120000 // long pause for scale softening
#define MAX_SETPOINT_VALUE 110 //Defines the max value of the setpoint
#define POWER_DRAW_ZERO 42 // sets the zero bar acs power value
#define EEPROM_RESET 251 //change this value if want to reset to defaults


//Init the thermocouples with the appropriate pins defined above with the prefix "thermo"
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
// EasyNextion object init
EasyNex myNex(Serial);
// RobotDYN Dimmer object init
dimmerLamp dimmer(dimmerPin); //initialise the dimmer on the chosen port

//##################__Transducer_stuff__##################################
const float voltageZero = 0.49; // the voltage output by the transducer at 0bar - aka our offset
float pressureValue; //variable to store the value coming from the pressure transducer

// setting the pump performance based on region
// uint8_t BAR_TO_DIMMER_OUTPUT[10] = {42,45,50,53,56,60,64,68,70,75}; // the dimmer setpoints for the 220v pumps
uint8_t BAR_TO_DIMMER_OUTPUT[10]; //={45,51,53,56,58,60,63,65,69,73}; // the dimmer setpoints for the 110v pumps


// Some vars are better global
volatile float kProbeReadValue;
unsigned long thermoTimer;
bool POWER_ON;
bool  descaleCheckBox;
bool  preinfusionState;
bool  pressureProfileState;
bool  warmupEnabled;
bool  flushEnabled;
bool  descaleEnabled;
bool preinfusionFinished;
volatile uint16_t  HPWR;
volatile uint16_t  HPWR_OUT;
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
uint16_t  EEP_SETPOINT = 1;
uint16_t  EEP_OFFSET = 20;
uint16_t  EEP_HPWR = 40;
uint16_t  EEP_M_DIVIDER = 60;
uint16_t  EEP_B_DIVIDER = 80;
uint16_t  EEP_P_START = 100;
uint16_t  EEP_P_FINISH = 120;
uint16_t  EEP_P_HOLD = 110;
uint16_t  EEP_P_LENGTH = 130;
uint16_t  EEP_PREINFUSION = 140;
uint16_t  EEP_P_PROFILE = 160;
uint16_t  EEP_PREINFUSION_SEC = 180;
uint16_t  EEP_PREINFUSION_BAR = 190;
uint16_t  EEP_PREINFUSION_SOAK = 170; 
uint16_t  EEP_REGPWR_HZ = 195;
uint16_t  EEP_WARMUP = 200;
uint16_t  EEP_HOME_ON_SHOT_FINISH = 205;
uint16_t  EEP_GRAPH_BREW = 210;


void setup() {
  
  Serial.begin(115200); // switching our board to the new serial speed
  
  // relay port init and set initial operating mode
  pinMode(relayPin, OUTPUT);
  pinMode(brewPin, INPUT_PULLUP);
  pinMode(steamPin, INPUT_PULLUP);
  // Chip side  HIGH/LOW  specification
  PORTB &= ~_BV(PB0);  // relayPin LOW
  
  // Will wait hereuntil full serial is established, this is done so the LCD fully initializes before passing the EEPROM values
  while (myNex.readNumber("safetyTempCheck") != 100 )
  {
    delay(500);
  }
  //If it's the first boot we'll need to set some defaults
  if (EEPROM.read(0) != EEPROM_RESET || EEPROM.read(EEP_SETPOINT) == 0 || EEPROM.read(EEP_SETPOINT) == 65535|| EEPROM.read(EEP_PREINFUSION_SOAK) == 65535) {
    Serial.println("SECU_CHECK FAILED! Applying defaults!");
    EEPROM.put(0, EEPROM_RESET);
    //The values can be modified to accomodate whatever system it tagets
    //So on first boot it writes and reads the desired system values
    EEPROM.put(EEP_SETPOINT, 100);
    EEPROM.put(EEP_OFFSET, 7);
    EEPROM.put(EEP_HPWR, 550);
    EEPROM.put(EEP_M_DIVIDER, 5);
    EEPROM.put(EEP_B_DIVIDER, 2);
    delay(5);
    EEPROM.put(EEP_PREINFUSION, 0);
    EEPROM.put(EEP_P_START, 9);
    EEPROM.put(EEP_P_FINISH, 6);
    EEPROM.put(EEP_P_PROFILE, 0);
    EEPROM.put(EEP_PREINFUSION_SEC, 8);
    EEPROM.put(EEP_PREINFUSION_BAR, 2);
    delay(5);
    EEPROM.put(EEP_REGPWR_HZ, 60);
    EEPROM.put(EEP_WARMUP, 0);
    EEPROM.put(EEP_GRAPH_BREW, 0);
    EEPROM.put(EEP_HOME_ON_SHOT_FINISH, 1);
    EEPROM.put(EEP_PREINFUSION_SOAK, 5);
    EEPROM.put(EEP_P_HOLD, 7);
    EEPROM.put(EEP_P_LENGTH, 10);
    EEPROM.put(EEP_GRAPH_BREW, 0);
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
  if ( init_val > 0 ) {
    myNex.writeNum("offSet", init_val);
    myNex.writeNum("moreTemp.n2.val", init_val);
  }
  EEPROM.get(EEP_HPWR, init_val);//reading HPWR value from eeprom
  if (  init_val > 0 ) {
    myNex.writeNum("hpwr", init_val);
    myNex.writeNum("moreTemp.n3.val", init_val);
  }
  EEPROM.get(EEP_M_DIVIDER, init_val);//reading main cycle div from eeprom
  if ( init_val > 1 ) {
    myNex.writeNum("mDiv", init_val);
    myNex.writeNum("moreTemp.n4.val", init_val);
  }
  EEPROM.get(EEP_B_DIVIDER, init_val);//reading brew cycle div from eeprom
  if (  init_val > 1 ) {
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
  if (!(init_val < 0)) {
    myNex.writeNum("piSec", init_val);
    myNex.writeNum("brewAuto.n0.val", init_val);
  }

  EEPROM.get(EEP_PREINFUSION_BAR, init_val);//reading preinfusion pressure value from eeprom
  if (  !(init_val < 0) && init_val <= 9 ) {
    myNex.writeNum("piBar", init_val);
    myNex.writeNum("brewAuto.n1.val", init_val);
  }
  EEPROM.get(EEP_PREINFUSION_SOAK, init_val);//reading preinfusion soak times value from eeprom
  if (!(init_val < 0)) {
    myNex.writeNum("piSoak", init_val);
    myNex.writeNum("brewAuto.n4.val", init_val);
  }
  // Region POWER value
  EEPROM.get(EEP_REGPWR_HZ, init_val);//reading region frequency value from eeprom
  if (  init_val == 50 || init_val == 60 ) {
    myNex.writeNum("regHz", init_val);
    // Setting the pump performance based on loaded region  settings
    switch (init_val) {
      case 50: // 240v / 50Hz
        BAR_TO_DIMMER_OUTPUT[0]=40;
        BAR_TO_DIMMER_OUTPUT[1]=45;
        BAR_TO_DIMMER_OUTPUT[2]=50;
        BAR_TO_DIMMER_OUTPUT[3]=53;
        BAR_TO_DIMMER_OUTPUT[4]=56;
        BAR_TO_DIMMER_OUTPUT[5]=60;
        BAR_TO_DIMMER_OUTPUT[6]=62;
        BAR_TO_DIMMER_OUTPUT[7]=65;
        BAR_TO_DIMMER_OUTPUT[8]=68;
        BAR_TO_DIMMER_OUTPUT[9]=70;
        break;
      case 60: // 120v / 60 Hz
        BAR_TO_DIMMER_OUTPUT[0]=45;
        BAR_TO_DIMMER_OUTPUT[1]=51;
        BAR_TO_DIMMER_OUTPUT[2]=53;
        BAR_TO_DIMMER_OUTPUT[3]=56;
        BAR_TO_DIMMER_OUTPUT[4]=58;
        BAR_TO_DIMMER_OUTPUT[5]=60;
        BAR_TO_DIMMER_OUTPUT[6]=63;
        BAR_TO_DIMMER_OUTPUT[7]=65;
        BAR_TO_DIMMER_OUTPUT[8]=70;
        BAR_TO_DIMMER_OUTPUT[9]=73;
        break;
      default: // smth went wrong the pump is set to 0 bar in all modes.
        break;
    }
  }

  // Brew page settings
  EEPROM.get(EEP_HOME_ON_SHOT_FINISH, init_val);//reading preinfusion pressure value from eeprom
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
  
  // Dimmer initialisation
  dimmer.begin(NORMAL_MODE, ON); //dimmer initialisation: name.begin(MODE, STATE)
  dimmer.setPower(BAR_TO_DIMMER_OUTPUT[9]);
  
  myNex.lastCurrentPageId = myNex.currentPageId;
  POWER_ON = true;
  thermoTimer = millis();
}

//##############################################################################################################################
//############################################________________MAIN______________################################################
//##############################################################################################################################


//Main loop where all the below logic is continuously run
void loop() {
  pageValuesRefresh();
  myNex.NextionListen();
  kThermoRead();
  modeSelect();
  lcdRefresh();
}

//##############################################################################################################################
//###########################################___________THERMOCOUPLE_READ________###############################################
//##############################################################################################################################

// K-TYPE thermocouple read function
void kThermoRead() { // Reading the thermocouple temperature
  // Reading the temperature every 350ms between the loops
  if ((millis() - thermoTimer) > GET_KTYPE_READ_EVERY){
    kProbeReadValue = thermocouple.readCelsius();  // Making sure we're getting a value
    while (kProbeReadValue <= 0.0 || kProbeReadValue == NAN || kProbeReadValue > 160.0) {
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      if ((millis() - thermoTimer) > GET_KTYPE_READ_EVERY){
        kProbeReadValue = thermocouple.readCelsius();  // Making sure we're getting a value
        thermoTimer = millis();
      }
    }
    thermoTimer = millis();
  }
}

//##############################################################################################################################
//############################################______PRESSURE_____TRANSDUCER_____################################################
//##############################################################################################################################
float getPressure() {  //returns sensor pressure data
    float voltage = (analogRead(pressurePin)*5.0)/1024.0; // finding the voltage representation of the current analog value
    float pressure_bar = (voltage-voltageZero)*3.0; // converting to bars of pressure
    return pressure_bar;
}


uint8_t setPressure(float wantedValue, uint8_t minVal, uint8_t maxVal) {
  static double refreshTimer;
  static float outputValue, prevOutputValue;
  float livePressure = getPressure();

  if (brewState() == 1 ) {
    if (livePressure < wantedValue) {
      if ((livePressure/wantedValue)>=1.0 ) outputValue--;
	  else if ((livePressure/wantedValue) > 0.5 && (livePressure/wantedValue) < 0.8 ) outputValue++;
	  else outputValue = BAR_TO_DIMMER_OUTPUT[uint8_t(wantedValue)];
	  constrain(outputValue,BAR_TO_DIMMER_OUTPUT[0],BAR_TO_DIMMER_OUTPUT[9]);
      prevOutputValue=outputValue;
      return outputValue;
    }
    else if (livePressure > wantedValue) {
	  if ((wantedValue/livePressure) > 0.5 && (wantedValue/livePressure) < 0.8 ) outputValue--;
	  else if ((wantedValue/livePressure) > 0.9 )outputValue++;
	  else outputValue = BAR_TO_DIMMER_OUTPUT[uint8_t(wantedValue)];
      constrain(outputValue,BAR_TO_DIMMER_OUTPUT[0],BAR_TO_DIMMER_OUTPUT[9]);
      prevOutputValue=outputValue;
      refreshTimer = millis();
      return outputValue;
    }
    else return prevOutputValue;
  }else return BAR_TO_DIMMER_OUTPUT[uint8_t(wantedValue)];
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
    if (selectedOperationalMode < 0 || selectedOperationalMode > 10) selectedOperationalMode = myNex.readNumber("modeSelect");

    myNex.lastCurrentPageId = myNex.currentPageId;
    POWER_ON = false;
  }
}

//#############################################################################################
//############################____OPERATIONAL_MODE_CONTROL____#################################
//#############################################################################################
void modeSelect() {
  switch (selectedOperationalMode) {
    case 0:
      if (steamState() == 0) justDoCoffee();
      else steamCtrl();
      break;
    case 1:
      if (steamState() == 0) preInfusion();
      else steamCtrl();
      break;
    case 2:
      if (steamState() == 0) autoPressureProfile();
      else steamCtrl();
      break;
    case 3:
      manualPressureProfile();
      break;
    case 4:
      if (steamState() == 0) {
        if(preinfusionFinished == false) preInfusion();
        else if(preinfusionFinished == true) autoPressureProfile();
      } else if (steamState() == 1) steamCtrl();
      break;
    case 5:
      if (steamState() == 0) justDoCoffee();
      else steamCtrl();
      break;
    case 6:
      deScale(descaleCheckBox);
      break;
    case 7:
      break;
    case 8:
      break;
    case 9:
      if (steamState() == 0) justDoCoffee();
      else steamCtrl();
      break;
    default:
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
  // Calculating the boiler heating power range based on the below input values
  HPWR_OUT = mapRange(kProbeReadValue, setPoint - 10, setPoint, HPWR, HPWR_LOW, 0);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, HPWR);  // limits range of sensor values to HPWR_LOW and HPWR

  if (brewState() == 1) {
    if (selectedOperationalMode == 0 || selectedOperationalMode == 9) {
      dimmer.setPower(BAR_TO_DIMMER_OUTPUT[9]);
      brewTimer(1);
    }
    myNex.writeNum("warmupState", 0);
  // Applying the HPWR_OUT variable as part of the relay switching logic
    if (kProbeReadValue < setPoint+0.5) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT/BrewCycleDivider);  // delaying the relayPin state change
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT); 
    }
  } else if (brewState() == 0) {
    brewTimer(0);
    if (kProbeReadValue < ((float)setPoint - 10.00)) {
      PORTB |= _BV(PB0);  // relayPin -> HIGH
    } else if (kProbeReadValue >= ((float)setPoint - 10.00) && kProbeReadValue < ((float)setPoint - 3.00)) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
    } else if ((kProbeReadValue >= ((float)setPoint - 3.00)) && (kProbeReadValue <= ((float)setPoint - 1.00))) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
    } else if ((kProbeReadValue >= ((float)setPoint - 0.5)) && kProbeReadValue < (float)setPoint) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT/2);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT*2);  // delaying the relayPin state for <HPWR_OUT> ammount of time
    } else {
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
    }
  } 
}

//#############################################################################################
//################################____STEAM_POWER_CONTROL____##################################
//#############################################################################################

void steamCtrl() {
  float boilerPressure = getPressure();

  if (brewState() == 0) {
    if (boilerPressure <= 9.0) {
      if ((kProbeReadValue > setPoint-10.00) && (kProbeReadValue <=155)) PORTB |= _BV(PB0);  // relayPin -> HIGH
      else PORTB &= ~_BV(PB0);  // relayPin -> LOW
    }else if(boilerPressure >=9.1) PORTB &= ~_BV(PB0);  // relayPin -> LOW
  }else PORTB &= ~_BV(PB0);  // relayPin -> LOW
}

//#############################################################################################
//################################____LCD_REFRESH_CONTROL___###################################
//#############################################################################################

void lcdRefresh() {
  // Updating the LCD every 300ms
  static unsigned long pageRefreshTimer;
  
  if (millis() - pageRefreshTimer > REFRESH_SCREEN_EVERY) {
    // myNex.writeNum("currentHPWR", HPWR_OUT);
    myNex.writeNum("pressure.val", int(getPressure()*10));
    myNex.writeNum("currentTemp",int(kProbeReadValue-offsetTemp));
    pageRefreshTimer = millis();
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
//###############################_____HELPER_FUCTIONS____######################################
//#############################################################################################

//Function to get the state of the brew switch button
//returns true or false based on the read P(power) value
bool brewState() {  //Monitors the current flowing through the ACS712 circuit and returns a value depending on the power value (P) the system draws
 return (digitalRead(brewPin) != LOW ) ? 0 : 1; // pin will be high when switch is off.
}


// Returns HIGH when switch is OFF and LOW when ON
// pin will be high when switch is off.
bool steamState() {
  return (digitalRead(steamPin) != LOW) ? 0 : 1;
}


bool brewTimer(bool c) { // small function for easier timer start/stop
  if ( c == 1) {  
    myNex.writeNum("timerState", 1);
  }else if( c == 0) { 
    myNex.writeNum("timerState", 0);
  }
}


double mapRange(double sourceNumber, double fromA, double fromB, double toA, double toB, int decimalPrecision ) {
  double deltaA = fromB - fromA;
  double deltaB = toB - toA;
  double scale  = deltaB / deltaA;
  double negA   = -1 * fromA;
  double offset = (negA * scale) + toA;
  double finalNumber = (sourceNumber * scale) + offset;
  int calcScale = (int) pow(10, decimalPrecision);
  return (double) round(finalNumber * calcScale) / calcScale;
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
          dimmer.setPower(BAR_TO_DIMMER_OUTPUT[2]);
          if (millis() - timer > DESCALE_PHASE1_EVERY) { //set dimmer power to max descale value for 10 sec
            if (currentCycleRead >=100) descaleFinished = true;
            blink = false;
            currentCycleRead = myNex.readNumber("j0.val");
            timer = millis();
          }
        }else {
          dimmer.setPower(BAR_TO_DIMMER_OUTPUT[1]);
          if (millis() - timer > DESCALE_PHASE2_EVERY) { //set dimmer power to min descale value for 20 sec
            blink = true;
            currentCycleRead++;
            if (currentCycleRead<100) myNex.writeNum("j0.val", currentCycleRead);
            timer = millis();
          }
        }
      }else {
        dimmer.setPower(BAR_TO_DIMMER_OUTPUT[0]);
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
      dimmer.setPower(BAR_TO_DIMMER_OUTPUT[0]);
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
  static double newBarValue;

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
      // dimmer.setPower(BAR_TO_DIMMER_OUTPUT[ppStartBar]);
      dimmer.setPower(setPressure(ppStartBar,ppStartBar,ppStartBar));
    } else if (phase_2 == true) { //enters pahse 2
      if (ppStartBar < ppFinishBar) { // Incremental profiling curve
        newBarValue = mapRange(millis(),timer,timer + (ppLength*1000),ppStartBar,ppFinishBar,1); //Used to calculate the pressure drop/raise during a @ppLength sec shot
        if (newBarValue < (double)ppStartBar) newBarValue = (double)ppStartBar;
        else if (newBarValue > (double)ppFinishBar) newBarValue = (double)ppFinishBar;
      }else if (ppStartBar > ppFinishBar) { // Decremental profiling curve
        newBarValue = mapRange(millis(),timer,timer + (ppLength*1000),ppStartBar,ppFinishBar,1); //Used to calculate the pressure drop/raise during a @ppLength sec shot
        if (newBarValue > (double)ppStartBar) newBarValue = (double)ppStartBar;
        else if (newBarValue < ppFinishBar) newBarValue = (double)ppFinishBar;      
      }else { // Flat line profiling
        newBarValue = mapRange(millis(),timer,timer + (ppLength*1000),ppStartBar,ppFinishBar,1); //Used to calculate the pressure drop/raise during a @ppLength sec shot
        if (newBarValue < (double)ppStartBar) newBarValue = (double)ppStartBar;
        else if (newBarValue > (double)ppFinishBar) newBarValue = (double)ppFinishBar;
      }
      dimmer.setPower(setPressure(newBarValue,ppStartBar,ppFinishBar));
    }
  }else if ( brewState() == 0 ) { 
    brewTimer(0);
    if (selectedOperationalMode == 1 ) dimmer.setPower(setPressure(ppStartBar,ppStartBar,ppFinishBar));
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
      dimmer.setPower(power_reading);
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
        dimmer.setPower(setPressure(preinfuseBar,preinfuseBar,preinfuseBar));
        if ((millis() - timer) >= (preinfuseTime*1000)) {
          blink = false;
          timer = millis();
        }
      }else {
        dimmer.setPower(setPressure(0,0,0));
        if ((millis() - timer) >= (preinfuseSoak*1000)) { 
          exitPreinfusion = true;
          blink = true;
          timer = millis();
        }
      }
      // myNex.writeStr("t11.txt",String(getPressure(),1));
    }else if(exitPreinfusion == true && selectedOperationalMode == 1){ // PI
      dimmer.setPower(setPressure(9,9,9));
      // myNex.writeNum("currentPressure",BAR_TO_DIMMER_OUTPUT[9]);
    }else if(exitPreinfusion == true && selectedOperationalMode == 4){ // PI + PP
      preinfusionFinished = true;
      dimmer.setPower(setPressure(ppStartBar,ppStartBar,ppFinishBar));
      // myNex.writeNum("currentPressure",BAR_TO_DIMMER_OUTPUT[ppStartBar]);
    }
  }else if ( brewState() == 0 ) { //resetting all the values
    brewTimer(0);
    dimmer.setPower(setPressure(preinfuseBar,preinfuseBar,preinfuseBar));
    // myNex.writeNum("currentPressure",BAR_TO_DIMMER_OUTPUT[preinfuseBar]);
    exitPreinfusion = false;
    timer = millis();
  }
 //keeping it at temp
  justDoCoffee();
}
