#include <RBDdimmer.h>
#include <EEPROM.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>
#include <ACS712.h>
// #include "pressureRead.h"



// Define our pins
#define thermoDO 4
#define thermoCS 5
#define thermoCLK 6
#define brewSwitchPin A0 // PD7
#define relayPin 8  // PB0
#define dimmerPin 9
#define pressurePin A1 

// Define some const values
#define GET_KTYPE_READ_EVERY 350 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#define REFRESH_SCREEN_EVERY 350 // Screen refresh interval (ms)
#define DIMMER_UPDATE_EVERY 1000 // Defines how often the dimmer gets calculated a new value during a brew cycle (ms)
#define DESCALE_PHASE1_EVERY 500 // short pump pulses during descale
#define DESCALE_PHASE2_EVERY 5000 // short pause for pulse effficience activation
#define DESCALE_PHASE3_EVERY 120000 // long pause for scale softening
#define MAX_SETPOINT_VALUE 110 //Defines the max value of the setpoint
#define PI_SOAK_FOR 3000 // sets the ammount of time the preinfusion soaking phase is going to last for (ms)


//Init the thermocouple with the appropriate pins defined above with the prefix "thermo"
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
// EasyNextion object init
EasyNex myNex(Serial);
//Init the ACS712 hall sensor
ACS712 sensor(ACS712_20A, brewSwitchPin);
// RobotDYN Dimmer object init
dimmerLamp dimmer(dimmerPin); //initialise the dimmer on the chosen port
//Transducer stuff
// transducer Sensor(pressurePin);

// Sensor.set_values(921.6,105.0,12);


//Change these values if your tests show the dimmer should be tuned
// BAR --0-|-1-|-2-|-3-|-4-|-5-|-6-|-7-|-8-|-9
// DIM -40-|48-|50-|52-|55-|60-|67-|72-|80-|97
uint8_t BAR_TO_DIMMER_OUTPUT[10]={40,48,50,52,55,60,67,72,80,95};

// Some vars are better global
volatile float currentTempReadValue;
unsigned long thermoTimer;
bool POWER_ON;
bool  descaleCheckBox;
bool  preinfusionState;
bool  pressureProfileState;
bool  warmupEnabled;
bool  flushEnabled;
bool  descaleEnabled;
bool  fineTempEnabled;
bool  valuesRefreshOnPageChange;
bool preinfusionFinished;
volatile uint16_t  HPWR;
volatile uint16_t  HPWR_OUT;
uint16_t  setPoint;
uint16_t  offsetTemp;
uint8_t  MainCycleDivider;
uint8_t  BrewCycleDivider;
uint8_t  preinfuseTime;
uint8_t preinfuseBar;
uint8_t ppStartBar;
uint8_t ppFinishBar;
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
uint16_t  EEP_PREINFUSION = 140;
uint16_t  EEP_P_PROFILE = 160;
uint16_t  EEP_PREINFUSION_SEC = 180;
uint16_t  EEP_PREINFUSION_BAR = 190;
uint16_t  EEP_REGPWR_HZ = 195;
uint16_t  EEP_WARMUP = 200;


void setup() {
  
  Serial.begin(115200); // switching our board to the new serial speed

  dimmer.begin(NORMAL_MODE, ON); //dimmer initialisation: name.begin(MODE, STATE)
  dimmer.setPower(BAR_TO_DIMMER_OUTPUT[9]);

  // Calibrating the hall current sensor
  sensor.calibrate();

  // relay port init and set initial operating mode
  pinMode(relayPin, OUTPUT);
  pinMode(brewSwitchPin, INPUT);

  // Chip side  HIGH/LOW  specification
  PORTB &= ~_BV(PB0);  // relayPin LOW

  // Will wait hereuntil full serial is established, this is done so the LCD fully initializes before passing the EEPROM values
  delay(500);


  //If it's the first boot we'll need to set some defaults
  if (EEPROM.read(0) != 251 || EEPROM.read(EEP_SETPOINT) == 0 || EEPROM.read(EEP_SETPOINT) == 65535) {
    Serial.println("SECU_CHECK FAILED! Applying defaults!");
    EEPROM.put(0, 251);
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
  if (  init_val > 1 ) {
    myNex.writeNum("ppStart", init_val);
    myNex.writeNum("brewAuto.n2.val", init_val);
  }

  EEPROM.get(EEP_P_FINISH, init_val);// reading pressure profile finish value from eeprom
  if (  init_val > 1 ) {
    myNex.writeNum("ppFin", init_val);
    myNex.writeNum("brewAuto.n3.val", init_val);
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
  if (  !(init_val < 0) && init_val < 11  ) {
    myNex.writeNum("piSec", init_val);
    myNex.writeNum("brewAuto.n0.val", init_val);
  }

  EEPROM.get(EEP_PREINFUSION_BAR, init_val);//reading preinfusion pressure value from eeprom
  if (  !(init_val < 0) && init_val < 9 ) {
    myNex.writeNum("piBar", init_val);
    myNex.writeNum("brewAuto.n1.val", init_val);
  }

// Region POWER value
  EEPROM.get(EEP_REGPWR_HZ, init_val);//reading region frequency value from eeprom
  if (  init_val == 50 || init_val == 60 ) {
    myNex.writeNum("regHz", init_val);
  }

// Warmup checkbox value
  EEPROM.get(EEP_WARMUP, init_val);//reading preinfusion pressure value from eeprom
  if (  init_val == 0 || init_val == 1 ) {
    myNex.writeNum("warmupState", init_val);
    myNex.writeNum("morePower.bt0.val", init_val);
  }

  myNex.lastCurrentPageId = myNex.currentPageId;
  delay(5);
  POWER_ON = true;
  thermoTimer = millis();
}

//##############################################################################################################################
//############################################________________MAIN______________################################################
//##############################################################################################################################


//Main loop where all the below logic is continuously run
void loop() {
  Power_ON_Values_Refresh();
  myNex.NextionListen();
  kThermoRead();
  modeSelect();
  lcdRefresh();
  pageValuesRefresh();
}

//##############################################################################################################################
//###########################################___________THERMOCOUPLE_READ________###############################################
//##############################################################################################################################

void kThermoRead() { // Reading the thermocouple temperature
  // Reading the temperature every 350ms between the loops
  if ((millis() - thermoTimer) > GET_KTYPE_READ_EVERY){
    currentTempReadValue = thermocouple.readCelsius();  // Making sure we're getting a value
    while (currentTempReadValue <= 0 || currentTempReadValue == NAN) {
      if ((millis() - thermoTimer) > GET_KTYPE_READ_EVERY){
        currentTempReadValue = thermocouple.readCelsius();  // Making sure we're getting a value
        thermoTimer = millis();
      }
    }
    thermoTimer = millis();
  }
}


//##############################################################################################################################
//############################################______POWER_ON_VALUES_REFRESH_____################################################
//##############################################################################################################################

void Power_ON_Values_Refresh() {  // Refreshing our values on first start

  if (POWER_ON == true) {
    
    setPoint = myNex.readNumber("setPoint");  // reading the setPoint value from the lcd
    offsetTemp = myNex.readNumber("offSet");  // reading the offset value from the lcd
    HPWR = myNex.readNumber("hpwr");  // reading the brew time delay used to apply heating in waves
    MainCycleDivider = myNex.readNumber("mDiv");  // reading the delay divider
    BrewCycleDivider = myNex.readNumber("bDiv");  // reading the delay divider
    delay(5);

    preinfusionState = myNex.readNumber("piState"); // reding the preinfusion state value which should be 0 or 1
    preinfuseTime = myNex.readNumber("piSec"); // pre-infusion duration
    preinfuseBar = myNex.readNumber("piBar"); //pre-infusion pressure value
    delay(5);
  
    pressureProfileState = myNex.readNumber("ppState"); // pressure profile state value which should be 0 or 1
    ppStartBar = myNex.readNumber("ppStart"); // pressure profile start pressure value
    ppFinishBar = myNex.readNumber("ppFin"); // pressure profile finish pressure value
    delay(5);

    regionHz = myNex.readNumber("regHz"); // regionVolts = myNex.readNumber("regVolt");
    warmupEnabled = myNex.readNumber("warmupState");
    delay(5);
    
    // MODE_SELECT should always be last
    selectedOperationalMode = myNex.readNumber("modeSelect");
    if (selectedOperationalMode < 0 || selectedOperationalMode > 10) selectedOperationalMode = myNex.readNumber("modeSelect");

    myNex.lastCurrentPageId = myNex.currentPageId;
    POWER_ON = false;
  }
}

//##############################################################################################################################
//############################################______PAGE_CHANGE_VALUES_REFRESH_____#############################################
//##############################################################################################################################

void pageValuesRefresh() {  // Refreshing our values on page changes

  if (myNex.currentPageId != myNex.lastCurrentPageId) {
    preinfusionState = myNex.readNumber("piState"); // reding the preinfusion state value which should be 0 or 1
    pressureProfileState = myNex.readNumber("ppState"); // reding the pressure profile state value which should be 0 or 1
    preinfuseTime = myNex.readNumber("piSec");
    preinfuseBar = myNex.readNumber("piBar"); 
    ppStartBar = myNex.readNumber("ppStart");
    ppFinishBar = myNex.readNumber("ppFin");
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
  }
}

//#############################################################################################
//############################____OPERATIONAL_MODE_CONTROL____#################################
//#############################################################################################
void modeSelect() {
  switch (selectedOperationalMode) {
    case 0:
      justDoCoffee();
      break;
    case 1:
      preInfusion();
      break;
    case 2:
      autoPressureProfile();
      break;
    case 3:
      manualPressureProfile();
      break;
    case 4:
      if(preinfusionFinished == false) preInfusion();
      else if(preinfusionFinished == true) autoPressureProfile();
      break;
    case 5:
      justDoCoffee();
      break;
    case 6:
      deScale(descaleCheckBox);
      break;
    default:
      justDoCoffee();
      break;
  }
}

//#############################################################################################
//#########################____NO_OPTIONS_ENABLED_POWER_CONTROL____############################
//#############################################################################################
void justDoCoffee() {
  uint8_t HPWR_LOW = HPWR/MainCycleDivider;
  // Calculating the boiler heating power range based on the below input values
  HPWR_OUT = map(currentTempReadValue, setPoint - 10, setPoint, HPWR, HPWR_LOW);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, HPWR);  // limits range of sensor values to HPWR_LOW and HPWR

  if (brewState() == 1) {
    dimmer.setPower(BAR_TO_DIMMER_OUTPUT[9]);
    brewTimer(1);
    myNex.writeNum("warmupState", 0);
  // Applying the HPWR_OUT variable as part of the relay switching logic
    if (currentTempReadValue < setPoint+0.5) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT/BrewCycleDivider);  // delaying the relayPin state change
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT); 
    }
  } else if (brewState() == 0) {
    brewTimer(0);
    if (currentTempReadValue < ((float)setPoint - 10.00)) {
      PORTB |= _BV(PB0);  // relayPin -> HIGH
    } else if (currentTempReadValue >= ((float)setPoint - 10.00) && currentTempReadValue < ((float)setPoint - 3.00)) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
    } else if ((currentTempReadValue >= ((float)setPoint - 3.00)) && (currentTempReadValue <= ((float)setPoint - 1.00))) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
    } else if ((currentTempReadValue >= ((float)setPoint - 0.50)) && currentTempReadValue < (float)setPoint) {
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
//################################____HEAT_POWER_CONTROL____###################################
//#############################################################################################
void heatCtrl() {
  uint8_t HPWR_LOW= HPWR/MainCycleDivider;
  // Calculating the boiler heating power range based on the below input values
  HPWR_OUT = map(currentTempReadValue, setPoint - 10, setPoint, HPWR, HPWR_LOW);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, HPWR);  // limits range of sensor values to between HPWR_LOW and HPWR

  if (brewState() == 1) {
  // Applying the HPWR_OUT variable as part of the relay switching logic
    if (currentTempReadValue < setPoint+0.5) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT/BrewCycleDivider);  // delaying the relayPin state change
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT); 
    }
    myNex.writeNum("warmupState", 0);
  } else if (brewState() == 0) {
    brewTimer(0);
    if (currentTempReadValue < ((float)setPoint - 10.00)) {
      PORTB |= _BV(PB0);  // relayPin -> HIGH
    } else if (currentTempReadValue >= ((float)setPoint - 10.00) && currentTempReadValue < ((float)setPoint - 3.00)) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
    } else if ((currentTempReadValue >= ((float)setPoint - 3.00)) && (currentTempReadValue <= ((float)setPoint - 1.00))) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
    } else if ((currentTempReadValue <= ((float)setPoint - 0.25)) && currentTempReadValue < (float)setPoint) {
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
//################################____LCD_REFRESH_CONTROL___###################################
//#############################################################################################

void lcdRefresh() {
  // Updating the LCD every 300ms
  static unsigned long pageRefreshTimer = millis();
    
  if (millis() - pageRefreshTimer > REFRESH_SCREEN_EVERY) {
    myNex.writeNum("currentHPWR", HPWR_OUT);      
    myNex.writeNum("currentTemp",int(currentTempReadValue-offsetTemp));
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
      // Saving ppStart and ppFin
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
      if ( valueToSave >= 0 ) {
        EEPROM.put(EEP_PREINFUSION_BAR, valueToSave);
        allValuesUpdated++;
      }else {}
      if (allValuesUpdated == 6) {
        allValuesUpdated=0;
        myNex.writeStr("popupMSG.t0.txt","UPDATE SUCCESSFUL!");
      }else myNex.writeStr("popupMSG.t0.txt","ERROR!");
      myNex.writeStr("page popupMSG");
      break;
    case 4:
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
  uint16_t P;
  // Checking which region we're running in so the right formula can be applied
  if (regionHz < 55 ) P = 240 * sensor.getCurrentAC();
  else if (regionHz > 55 ) P = 120 * sensor.getCurrentAC(regionHz);
  // Returnig "true" or "false" as the function response
  if ( P >= 45 ) return 1;
  else return 0;
}


bool brewTimer(bool c) { // small function for easier timer start/stop
  if ( c == 1) {  
    myNex.writeNum("timerState", 1);
  }else if( c == 0) { 
    myNex.writeNum("timerState", 0);
  }
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
    heatCtrl(); //keeping it at temp
  }
}


//#############################################################################################
//###############################____PRESSURE_CONTROL____######################################
//#############################################################################################


// Pressure profiling function, uses dimmer to dim the pump 
// as time passes, starts dimming at about X seconds mark 
// goes from 9bar to the lower threshold set in settings(default 6bar)
void autoPressureProfile() {
  static bool setPerformed = 0, phase_1 = 1, phase_2 = 0, updateTimer = 1;
  static unsigned long timer = millis();
  static uint8_t dimmerOutput;
  static uint8_t dimmerNewPowerVal;

  if (brewState() == 1) { //runs this only when brew button activated and pressure profile selected  
    if (updateTimer == 1) {
      timer = millis();
      updateTimer = 0;
    }
    if (phase_1 == true) { //enters phase 1
      if ((millis() - timer)>5000) { // the actions of this if block are run after 4 seconds have passed since starting brewing
        phase_1 = 0;
        phase_2 = 1;
        timer = millis();
      }
      brewTimer(1);
      dimmer.setPower(BAR_TO_DIMMER_OUTPUT[ppStartBar]);
      myNex.writeNum("currentPressure",BAR_TO_DIMMER_OUTPUT[ppStartBar]);
    } else if (phase_2 == true) { //enters pahse 2
      if (millis() - timer > DIMMER_UPDATE_EVERY) { // runs the below block every second
        if (BAR_TO_DIMMER_OUTPUT[ppStartBar] > BAR_TO_DIMMER_OUTPUT[ppFinishBar]) {
          dimmerOutput+=round(BAR_TO_DIMMER_OUTPUT[ppStartBar]/BAR_TO_DIMMER_OUTPUT[ppFinishBar])*2;
          dimmerNewPowerVal=BAR_TO_DIMMER_OUTPUT[ppStartBar]-dimmerOutput; //calculates a new dimmer power value every second given the max and min
          if (dimmerNewPowerVal<BAR_TO_DIMMER_OUTPUT[ppFinishBar]) dimmerNewPowerVal=BAR_TO_DIMMER_OUTPUT[ppFinishBar];  // limits range of sensor values between BAR_TO_DIMMER_OUTPUT[ppStartBar] and BAR_TO_DIMMER_OUTPUT[ppFinishBar]
          dimmer.setPower(dimmerNewPowerVal);
          myNex.writeNum("currentPressure",dimmerNewPowerVal);
        }else if (BAR_TO_DIMMER_OUTPUT[ppStartBar] < BAR_TO_DIMMER_OUTPUT[ppFinishBar]) {
          dimmerOutput+=round(BAR_TO_DIMMER_OUTPUT[ppFinishBar]/BAR_TO_DIMMER_OUTPUT[ppStartBar])*2;
          dimmerNewPowerVal = map(dimmerOutput, 0, 100, BAR_TO_DIMMER_OUTPUT[ppStartBar], BAR_TO_DIMMER_OUTPUT[ppFinishBar]); //calculates a new dimmer power value every second given the max and min
          if (dimmerNewPowerVal>BAR_TO_DIMMER_OUTPUT[ppFinishBar]) dimmerNewPowerVal=BAR_TO_DIMMER_OUTPUT[ppFinishBar];  // limits range of sensor values between BAR_TO_DIMMER_OUTPUT[ppStartBar] and BAR_TO_DIMMER_OUTPUT[ppFinishBar]
          dimmer.setPower(dimmerNewPowerVal);
          myNex.writeNum("currentPressure",dimmerNewPowerVal);
        }
        timer = millis();
      } 
    }
  }else if ( brewState() == 0 ) { 
    brewTimer(0);
    if (selectedOperationalMode == 1 ) {
      dimmer.setPower(BAR_TO_DIMMER_OUTPUT[ppStartBar]);
      myNex.writeNum("currentPressure",ppStartBar);
    }else if (selectedOperationalMode == 4 ) preinfusionFinished = false;
    timer = millis();
    phase_2 = false;
    phase_1=true;
    dimmerOutput=0;
    dimmerNewPowerVal=0;
    updateTimer = 1;
  }
  heatCtrl(); // Keep that water at temp
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
  heatCtrl();
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
        dimmer.setPower(BAR_TO_DIMMER_OUTPUT[preinfuseBar]);
        myNex.writeNum("currentPressure",BAR_TO_DIMMER_OUTPUT[preinfuseBar]);
        if ((millis() - timer) > (preinfuseTime*1000)) {
          // brewTimer(0);
          blink = false;
          timer = millis();
        }
      }else {
        // brewTimer(0);
        dimmer.setPower(BAR_TO_DIMMER_OUTPUT[0]);
        if (millis() - timer > PI_SOAK_FOR) { 
          exitPreinfusion = true;
          blink = true;
          timer = millis();
        }
      }
    }else if(exitPreinfusion == true && selectedOperationalMode == 1){ // just pre-infusion
      // brewTimer(1);
      dimmer.setPower(BAR_TO_DIMMER_OUTPUT[9]);
      myNex.writeNum("currentPressure",BAR_TO_DIMMER_OUTPUT[9]);
    }else if(exitPreinfusion == true && selectedOperationalMode == 4){ // pre-infusion with pressure profiling on
      // brewTimer(0);
      preinfusionFinished = true;
      dimmer.setPower(BAR_TO_DIMMER_OUTPUT[ppStartBar]);
      myNex.writeNum("currentPressure",BAR_TO_DIMMER_OUTPUT[ppStartBar]);
    }
  }else if ( brewState() == 0 ) { //resetting all the values
    brewTimer(0);
    dimmer.setPower(BAR_TO_DIMMER_OUTPUT[preinfuseBar]);
    myNex.writeNum("currentPressure",BAR_TO_DIMMER_OUTPUT[preinfuseBar]);
    exitPreinfusion = false;
    timer = millis();
  }
  heatCtrl(); //keeping it at temp
}
