#include <RBDdimmer.h>
#include <EEPROM.h>
#include <trigger.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>
#include <ACS712.h>




// Define our pins
#define thermoDO 4
#define thermoCS 5
#define thermoCLK 6
#define brewSwitchPin A0 // PD7
#define relayPin 8  // PB0
#define dimmerPin 9

// Define some const values
#define GET_KTYPE_READ_EVERY 350 // thermocouple data read interval not recommended to be changed to lower than 250 (ms)
#define REFRESH_SCREEN_EVERY 350 // Screen refresh interval (ms)
#define DIMMER_UPDATE_EVERY 1000 // Defines how often the dimmer gets calculated a new value during a brew cycle (ms)
#define DESCALE_PHASE1_EVERY 500 // short pump pulses during descale
#define DESCALE_PHASE2_EVERY 5000 // short pause for pulse effficience activation
#define DESCALE_PHASE3_EVERY 120000 // long pause for scale softening
#define MAX_SETPOINT_VALUE 110 //Defines the max value of the setpoint
#define PI_SOAK_FOR 3000 // sets the ammount of time the preinfusion soaking phase is going to last for (ms)
#define dimmerMinPowerValue 40
#define dimmerMaxPowerValue 97
#define dimmerDescaleMinValue 30
#define dimmerDescaleMaxValue 47


//Init the thermocouple with the appropriate pins defined above with the prefix "thermo"
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
// EasyNextion object init
EasyNex myNex(Serial);
//Init the ACS712 hall sensor
ACS712 sensor(ACS712_20A, brewSwitchPin);
// RobotDYN Dimmer object init
dimmerLamp dimmer(dimmerPin); //initialise the dimmer on the chosen port



//Change these values if your tests show the dimmer should be tuned
// BAR --0-|-1-|-2-|-3-|-4-|-5-|-6-|-7-|-8-|-9
// DIM -40-|48-|50-|52-|55-|60-|67-|72-|80-|97
uint8_t BAR_TO_DIMMER_OUTPUT[10]={40,48,50,52,55,60,67,72,80,97};

// Some vars are better global
volatile float currentTempReadValue = 0.0;
volatile float lastReadTempValue = 0.0;
unsigned long thermoTimer = millis();
bool POWER_ON;
bool  descaleCheckBox;
bool  preinfusionCheckBox;
bool  pressureProfileCheckBox;
bool  warmupCheckBox;
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
uint8_t ppressureProfileStartBar;
uint8_t ppressureProfileFinishBar;
uint8_t selectedOperationalMode;
uint8_t regionVolts;
uint8_t regionHz;

// Declaring local vars
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
uint16_t  EEP_REGPWR_V = 195;
uint16_t  EEP_REGPWR_HZ = 200;
uint16_t  EEP_DESCALE = 205;
uint16_t  EEP_WARMUP = 210;


void setup() {
  
  Serial.begin(115200); // switching our board to the new serial speed

  // To debug correct work of the below feature later
  // attachInterrupt(digitalPinToInterrupt(dimmerPin), dimmerInterrupt, CHANGE);

  dimmer.begin(NORMAL_MODE, ON); //dimmer initialisation: name.begin(MODE, STATE)
  dimmer.setPower(dimmerMaxPowerValue);

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
  if (EEPROM.read(0) != 253 || EEPROM.read(EEP_SETPOINT) == NULL || EEPROM.read(EEP_SETPOINT) == 65535) {
    Serial.println("SECU_CHECK FAILED! Applying defaults!");
    EEPROM.put(0, 253);
    //only written once as 0
    EEPROM.put(EEP_DESCALE, 0);
    
    //these can change
    EEPROM.put(EEP_SETPOINT, 101);
    EEPROM.put(EEP_OFFSET, 7);
    EEPROM.put(EEP_HPWR, 550);
    EEPROM.put(EEP_M_DIVIDER, 5);
    EEPROM.put(EEP_B_DIVIDER, 2);
    EEPROM.put(EEP_PREINFUSION, 0);
    EEPROM.put(EEP_P_START, 9);
    EEPROM.put(EEP_P_FINISH, 5);
    EEPROM.put(EEP_P_PROFILE, 0);
    EEPROM.put(EEP_PREINFUSION_SEC, 8);
    EEPROM.put(EEP_PREINFUSION_BAR, 2);
    EEPROM.put(EEP_REGPWR_V, 230);
    EEPROM.put(EEP_REGPWR_HZ, 50);
    EEPROM.put(EEP_WARMUP, 1);
  }
  // Applying our saved EEPROM saved values
  uint16_t init_val;
  // Loading the saved values fro EEPROM and sending them to the LCD

  EEPROM.get(EEP_SETPOINT, init_val);// reading setpoint value from eeprom
  if ( init_val > 0 ) myNex.writeNum("page1.n0.val", init_val);

  EEPROM.get(EEP_OFFSET, init_val); // reading offset value from eeprom
  if ( init_val > 0 ) myNex.writeNum("page1.n1.val", init_val);

  EEPROM.get(EEP_HPWR, init_val);//reading HPWR value from eeprom
  if (  init_val > 0 ) myNex.writeNum("page1.n2.val", init_val);

  EEPROM.get(EEP_M_DIVIDER, init_val);//reading main cycle div from eeprom
  if ( init_val > 1 ) myNex.writeNum("page1.n4.val", init_val);

  EEPROM.get(EEP_B_DIVIDER, init_val);//reading brew cycle div from eeprom
  if (  init_val > 1 ) myNex.writeNum("page1.n3.val", init_val);

  EEPROM.get(EEP_P_START, init_val);//reading pressure profile start value from eeprom
  if (  init_val > 1 ) {
    myNex.writeNum("page2.n0.val", init_val);
    myNex.writeNum("page2.pps_var.val", BAR_TO_DIMMER_OUTPUT[init_val]);
  }

  EEPROM.get(EEP_P_FINISH, init_val);// reading pressure profile finish value from eeprom
  if (  init_val > 1 ) {
    myNex.writeNum("page2.n1.val", init_val);
    myNex.writeNum("page2.ppf_var.val", BAR_TO_DIMMER_OUTPUT[init_val]);
  }

  EEPROM.get(EEP_DESCALE, init_val);//reading preinfusion checkbox value from eeprom
  if (  !(init_val < 0) && init_val < 2 ) myNex.writeNum("page2.c1.val", init_val);

  EEPROM.get(EEP_PREINFUSION, init_val);//reading preinfusion checkbox value from eeprom
  if (  !(init_val < 0) && init_val < 2 ) myNex.writeNum("page2.c0.val", init_val);

  EEPROM.get(EEP_P_PROFILE, init_val);//reading pressure profile checkbox value from eeprom
  if (  !(init_val < 0) && init_val < 2 ) {
    myNex.writeNum("page2.c2.val", init_val);
    if (init_val == 1) myNex.writeNum("page2.c3.val",1);
  }

  EEPROM.get(EEP_PREINFUSION_SEC, init_val);//reading preinfusion time value from eeprom
  if (  !(init_val < 0) && init_val < 11  ) myNex.writeNum("page2.h1.val", init_val);

  EEPROM.get(EEP_PREINFUSION_BAR, init_val);//reading preinfusion pressure value from eeprom
  if (  !(init_val < 0) && init_val < 98 ) myNex.writeNum("page2.preinf_pwr.val", init_val);

// Region POWER values
  EEPROM.get(EEP_REGPWR_V, init_val);//reading preinfusion pressure value from eeprom
  if (  !(init_val < 0) && init_val < 250 ) myNex.writeNum("page3.n0.val", init_val);
  EEPROM.get(EEP_REGPWR_HZ, init_val);//reading preinfusion pressure value from eeprom
  if (  !(init_val < 0) && init_val < 61 ) myNex.writeNum("page3.n1.val", init_val);
// Warmup checkbox value
  EEPROM.get(EEP_WARMUP, init_val);//reading preinfusion pressure value from eeprom
  if (  !(init_val >= 0) && init_val <= 1 ) myNex.writeNum("page3.c11.val", init_val);

//loading the correct operating mode according to the previously saved values
  if (myNex.readNumber("page2.c0.val")==1 && myNex.readNumber("page2.c2.val")==0 && myNex.readNumber("page2.c1.val")==0) myNex.writeNum("page0.mode_select.val",0);
  if (myNex.readNumber("page2.c0.val")==0 && myNex.readNumber("page2.c2.val")==1 && myNex.readNumber("page2.c1.val")==0) myNex.writeNum("page0.mode_select.val",1);
  if (myNex.readNumber("page2.c0.val")==1 && myNex.readNumber("page2.c2.val")==1 && myNex.readNumber("page2.c1.val")==0) myNex.writeNum("page0.mode_select.val",4);
  if (myNex.readNumber("page2.c1.val")==1) myNex.writeNum("page0.mode_select.val",3);
  if (myNex.readNumber("page2.c0.val")==0 && myNex.readNumber("page2.c2.val")==0 && myNex.readNumber("page2.c1.val")==0) myNex.writeNum("page0.mode_select.val",10);

  myNex.writeStr("page 0");
  myNex.lastCurrentPageId = 1;
  delay(5);
  POWER_ON = true;
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
  screenRefresh();
  pageValuesRefresh();
}

//##############################################################################################################################
//###########################################___________THERMOCOUPLE_READ________###############################################
//##############################################################################################################################

void kThermoRead() { // Reading the thermocouple temperature
  // Reading the temperature every 350ms between the loops
  if ((millis() - thermoTimer) > GET_KTYPE_READ_EVERY){
    currentTempReadValue = thermocouple.readCelsius();
    if (currentTempReadValue < 0 || currentTempReadValue == NAN) currentTempReadValue = thermocouple.readCelsius();  // Making sure we're getting a value
    thermoTimer = millis();
  }
}


//##############################################################################################################################
//############################################______POWER_ON_VALUES_REFRESH_____################################################
//##############################################################################################################################

void Power_ON_Values_Refresh() {  // Refreshing our values on first start

  if (POWER_ON == true) {
    
    ReadAagain_1:
    // Making sure the serial communication finishes sending all the values
    setPoint = myNex.readNumber("page1.n0.val");  // reading the setPoint value from the lcd
    if ( setPoint == NULL || setPoint < 0 || setPoint > MAX_SETPOINT_VALUE ) {
      myNex.writeNum("page0.n2.val",1);
      goto ReadAagain_1;
    }//setPoint = myNex.readNumber("page1.n0.val");
    delay(20);

    ReadAagain_2:
    offsetTemp = myNex.readNumber("page1.n1.val");  // reading the offset value from the lcd
    if (offsetTemp ==  NULL || offsetTemp < 0 ) {
      myNex.writeNum("page0.n2.val",2);
      goto ReadAagain_2;
    }//offsetTemp = myNex.readNumber("page1.n1.val");
    delay(20);

    ReadAagain_3:
    HPWR = myNex.readNumber("page1.n2.val");  // reading the brew time delay used to apply heating in waves
    if ( HPWR == NULL || HPWR < 0 ) {
      myNex.writeNum("page0.n2.val",3);
      goto ReadAagain_3;
    }//HPWR = myNex.readNumber("page1.n2.val");
    delay(20);

    ReadAagain_4:
    MainCycleDivider = myNex.readNumber("page1.n4.val");  // reading the delay divider
    if ( MainCycleDivider == NULL || MainCycleDivider < 1 ) {
      myNex.writeNum("page0.n2.val",4);
      goto ReadAagain_4;
    }//MainCycleDivider = myNex.readNumber("page1.n4.val");
    delay(20);
    ReadAagain_5:
    BrewCycleDivider = myNex.readNumber("page1.n3.val");  // reading the delay divider
    if ( BrewCycleDivider == NULL || BrewCycleDivider < 1  ) {
      myNex.writeNum("page0.n2.val",5);
      goto ReadAagain_5;
    }//BrewCycleDivider = myNex.readNumber("page1.n3.val");
    delay(20);

    ReadAagain_6:
    // reding the descale value which should be 0 or 1
    descaleCheckBox = myNex.readNumber("page2.c1.val");
    if ( descaleCheckBox < 0 || descaleCheckBox > 1 ) {
      myNex.writeNum("page0.n2.val",6);
      goto ReadAagain_6;
    }//descaleCheckBox = myNex.readNumber("page2.c1.val");
    delay(20);

    ReadAagain_7:
    // reding the preinfusion value which should be 0 or 1
    preinfusionCheckBox = myNex.readNumber("page2.c0.val");
    if ( preinfusionCheckBox < 0 || preinfusionCheckBox > 1 ){
      myNex.writeNum("page0.n2.val",7);
      goto ReadAagain_7;//preinfusionCheckBox = myNex.readNumber("page2.c0.val");
    }
    delay(20);

    ReadAagain_8:
    pressureProfileCheckBox = myNex.readNumber("page2.c2.val");
    if ( pressureProfileCheckBox < 0 || pressureProfileCheckBox > 1 ) {
      myNex.writeNum("page0.n2.val",8);
      goto ReadAagain_8; //pressureProfileCheckBox = myNex.readNumber("page2.c2.val");
    }
    if ( pressureProfileCheckBox > 0) myNex.writeNum("page2.c3.val",1); //enabling auto pressure profiling as a default when pressure profile checkbox selected
    delay(20);

    ReadAagain_9:
    preinfuseTime = myNex.readNumber("page2.h1.val");
    if (preinfuseTime < 0 || preinfuseTime > 10) {
      myNex.writeNum("page0.n2.val",9);
      goto ReadAagain_9;//preinfuseTime = myNex.readNumber("page2.h1.val");
    }
    delay(20);

    ReadAagain_10:
    preinfuseBar = myNex.readNumber("page2.preinf_pwr.val");
    if (preinfuseBar < 0 || preinfuseBar > 97) {
      myNex.writeNum("page0.n2.val",10);
      goto ReadAagain_10;//preinfuseBar = myNex.readNumber("page2.preinf_pwr.val");
    }
    delay(20);

    ReadAagain_11:    
    ppressureProfileStartBar = myNex.readNumber("page2.pps_var.val");
    ppressureProfileFinishBar = myNex.readNumber("page2.ppf_var.val");
    if (ppressureProfileStartBar < 0 || ppressureProfileStartBar > 97 || ppressureProfileFinishBar < 0 || ppressureProfileFinishBar > 97) {
      myNex.writeNum("page0.n2.val",11); 
      goto ReadAagain_11;//ppressureProfileFinishBar = myNex.readNumber("page2.ppf_var.val");
    }
    delay(20);
    ReadAagain_12:
    regionVolts = myNex.readNumber("page3.n0.val");
    regionHz = myNex.readNumber("page3.n1.val");
    if (regionVolts < 0 || regionVolts > 250 || regionHz < 0 || regionHz > 60) {
      myNex.writeNum("page0.n2.val",12); 
      goto ReadAagain_12;
    }
    delay(20);
    ReadAagain_13:
    warmupCheckBox = myNex.readNumber("page3.c11.val");
    if (warmupCheckBox < 0 || warmupCheckBox > 1) {
      myNex.writeNum("page0.c11.val",13); 
      goto ReadAagain_13;
    }
    delay(5);
    
    // MODE_SELECT should always be last
    selectedOperationalMode = myNex.readNumber("page0.mode_select.val");
    if (selectedOperationalMode < 0 || selectedOperationalMode > 10) selectedOperationalMode = myNex.readNumber("page0.mode_select.val");

    myNex.lastCurrentPageId = myNex.currentPageId;
    POWER_ON = false;
  }
}

//##############################################################################################################################
//############################################______PAGE_CHANGE_VALUES_REFRESH_____#############################################
//##############################################################################################################################

void pageValuesRefresh() {  // Refreshing our values on page changes

  if (myNex.currentPageId != myNex.lastCurrentPageId) {
      // Making sure the serial communication finishes sending all the values
    setPoint = myNex.readNumber("page1.n0.val");  // reading the setPoint value from the lcd
    if ( setPoint == NULL || setPoint < 0 || setPoint > MAX_SETPOINT_VALUE ) setPoint = myNex.readNumber("page1.n0.val");

    offsetTemp = myNex.readNumber("page1.n1.val");  // reading the offset value from the lcd
    if (offsetTemp ==  NULL || offsetTemp < 0 ) offsetTemp = myNex.readNumber("page1.n1.val");

    HPWR = myNex.readNumber("page1.n2.val");  // reading the brew time delay used to apply heating in waves
    if ( HPWR == NULL || HPWR < 0 ) HPWR = myNex.readNumber("page1.n2.val");

    MainCycleDivider = myNex.readNumber("page1.n4.val");  // reading the delay divider
    if ( MainCycleDivider == NULL || MainCycleDivider < 1 ) MainCycleDivider = myNex.readNumber("page1.n4.val");

    BrewCycleDivider = myNex.readNumber("page1.n3.val");  // reading the delay divider
    if ( BrewCycleDivider == NULL || BrewCycleDivider < 1  ) BrewCycleDivider = myNex.readNumber("page1.n3.val");

    // reding the descale value which should be 0 or 1
    descaleCheckBox = myNex.readNumber("page2.c1.val");
    if ( descaleCheckBox < 0 || descaleCheckBox > 1 || descaleCheckBox == NULL ) descaleCheckBox = myNex.readNumber("page2.c1.val");

    // reding the preinfusion value which should be 0 or 1
    preinfusionCheckBox = myNex.readNumber("page2.c0.val");
    if ( preinfusionCheckBox < 0 || preinfusionCheckBox > 1 ) preinfusionCheckBox = myNex.readNumber("page2.c0.val");

    pressureProfileCheckBox = myNex.readNumber("page2.c2.val");
    if ( pressureProfileCheckBox < 0 || pressureProfileCheckBox > 1 ) pressureProfileCheckBox = myNex.readNumber("page2.c2.val");

    preinfuseTime = myNex.readNumber("page2.h1.val");
    if (preinfuseTime < 0 || preinfuseTime > 10) preinfuseTime = myNex.readNumber("page2.h1.val");

    preinfuseBar = myNex.readNumber("page2.preinf_pwr.val");
    if (preinfuseBar < 0 || preinfuseBar > 97) preinfuseBar = myNex.readNumber("page2.preinf_pwr.val");
    
    ppressureProfileStartBar = myNex.readNumber("page2.pps_var.val");
    if (ppressureProfileStartBar < 0 || ppressureProfileStartBar > 97) ppressureProfileStartBar = myNex.readNumber("page2.pps_var.val");
    ppressureProfileFinishBar = myNex.readNumber("page2.ppf_var.val");
    if (ppressureProfileFinishBar < 0 || ppressureProfileFinishBar > 97) ppressureProfileFinishBar = myNex.readNumber("page2.ppf_var.val");

    // warmupCheckBox = myNex.readNumber("page3.c11.val");
    // if (warmupCheckBox < 0 || warmupCheckBox > 1) myNex.readNumber("page3.c11.val");

    // MODE_SELECT should always be last
    selectedOperationalMode = myNex.readNumber("page0.mode_select.val");
    if (selectedOperationalMode < 0 || selectedOperationalMode > 10) selectedOperationalMode = myNex.readNumber("page0.mode_select.val");

    myNex.lastCurrentPageId = myNex.currentPageId;
  }
}

//#############################################################################################
//############################____OPERATIONAL_MODE_CONTROL____#################################
//#############################################################################################
void modeSelect() {

  switch (selectedOperationalMode) {
    case 0:
      preInfusion(preinfusionCheckBox);
      break;
    case 1:
      autoPressureProfile();
      break;
    case 2:
      manualPressureProfile();
      break;
    case 3:
      deScale(descaleCheckBox);
      break;
    case 4:
      if(preinfusionFinished==false){
        preInfusion(preinfusionCheckBox);
      }else {
        autoPressureProfile();
      }
      break;
    default:
      justDoCoffee();
      break;
  }
}

//#############################################################################################
//#########################____NO_OPTIONS_ENABLED_POWER_CONTROL____############################
//#############################################################################################
// The temperature, preinfusion, dimming, LCD update, etc control logic is all in the below functions
void justDoCoffee() {
  uint8_t HPWR_LOW= HPWR/MainCycleDivider;
  // Calculating the boiler heating power range based on the below input values
  HPWR_OUT = map(currentTempReadValue, setPoint - 10, setPoint, HPWR, HPWR_LOW);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, HPWR);  // limits range of sensor values to HPWR_LOW and HPWR

  if (brewState() == true) {
    dimmer.setPower(dimmerMaxPowerValue);
    brewTimer(1);
  // Applying the HPWR_OUT variable as part of the relay switching logic
    if (currentTempReadValue < setPoint+0.5) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT/BrewCycleDivider);  // delaying the relayPin state change
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT); 
    }
  } else {
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

  if (brewState() == true) {
  // Applying the HPWR_OUT variable as part of the relay switching logic
    if (currentTempReadValue < setPoint+0.25) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT/BrewCycleDivider);  // delaying the relayPin state change
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT); 
    }
  } else {
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
void screenRefresh() {
  // myNex.currentPageId = myNex.readNumber("dp");
  // Updating the LCD every 300ms
    char waterTempPrint[6];
    static unsigned long pageRefreshTimer = millis();
    static bool steam_reset = 0;
    
  if (millis() - pageRefreshTimer > REFRESH_SCREEN_EVERY) {
    if (currentTempReadValue < 115.00 ) { //LCD update behaviour  when temps under 115C
      if (steam_reset == 1) {
        myNex.writeNum("page0.var_steam.val", 0); //Resetting the "STEAMING!" message
        steam_reset = 0;
      }
      myNex.writeNum("page0.n0.val", HPWR_OUT);      
    } else if (currentTempReadValue > 115.00 && currentTempReadValue < 170.00) { //Updating the LCD during steaming
      if (steam_reset == 0) {
        myNex.writeNum("page0.var_steam.val", 1);
        steam_reset = 1;
      }
    }
    // float tmp1 = currentTempReadValue - (float)offsetTemp;
    dtostrf(currentTempReadValue - (float)offsetTemp, 6, 2, waterTempPrint); // converting values with floating point to string
    myNex.writeStr("page0.t0.txt", waterTempPrint);  // Printing the current water temp values to the display

    //#####################________DEBUG_START_______########################
    //#######################################################################
    //myNex.writeNum("page0.n1.val", regionHz); //debug
    // if (regionVolts > 200) {
    //   float P = regionVolts * sensor.getCurrentAC();
    //   myNex.writeNum("page0.n2.val", P);
    // }else if (regionVolts < 200) {
    //   float P = regionVolts * sensor.getCurrentAC(regionHz);
    //   myNex.writeNum("page0.n2.val", P);
    // }
    //#########################______END_________############################
    lastReadTempValue = currentTempReadValue;
    pageRefreshTimer = millis();
  }
}

//#############################################################################################
//###################################____SAVE_BUTTON____#######################################
//#############################################################################################
// Save the desired temp values to EEPROM
void trigger1() {
  
  uint16_t savedBoilerTemp = myNex.readNumber("page1.n0.val");
  uint8_t savedOffsetTemp = myNex.readNumber("page1.n1.val");
  uint16_t savedHPWR = myNex.readNumber("page1.n2.val");
  uint8_t savedMainCycleDivider = myNex.readNumber("page1.n4.val");
  uint8_t savedBrewCycleDivider = myNex.readNumber("page1.n3.val");
  uint8_t savedPPStart = myNex.readNumber("page2.n0.val");
  uint8_t savedPPFinish = myNex.readNumber("page2.n1.val");
  uint8_t savedPreinfusion = myNex.readNumber("page2.c0.val");
  uint8_t savedPProfile = myNex.readNumber("page2.c2.val");
  uint8_t savedPreinfSec = myNex.readNumber("page2.h1.val");
  uint8_t savedPreinfBar = myNex.readNumber("page2.preinf_pwr.val");
  uint8_t savedRegVolt = myNex.readNumber("page3.n0.val");
  uint8_t savedRegHz = myNex.readNumber("page3.n1.val");
  uint8_t allValuesUpdated = 0;


  // Reading the LCD side set values
  if (savedBoilerTemp != NULL && savedBoilerTemp > 0) { 
    EEPROM.put(EEP_SETPOINT, savedBoilerTemp);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt","BOILER TEMP ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedOffsetTemp != NULL && savedOffsetTemp > 0) {
    EEPROM.put(EEP_OFFSET, savedOffsetTemp);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", "OFFSET ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedHPWR != NULL && savedHPWR > 0) {
    EEPROM.put(EEP_HPWR, savedHPWR);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", "HPWR ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedMainCycleDivider != NULL && savedMainCycleDivider >= 1) {
    EEPROM.put(EEP_M_DIVIDER, savedMainCycleDivider);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", "M_DIV ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedBrewCycleDivider != NULL && savedBrewCycleDivider >= 1) {
    EEPROM.put(EEP_B_DIVIDER, savedBrewCycleDivider);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", "B_DIV ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedPPStart != NULL && savedPPStart >= 1) {
    EEPROM.put(EEP_P_START, savedPPStart);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", "PP Start ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedPPFinish != NULL && savedPPFinish >= 1) {
    EEPROM.put(EEP_P_FINISH, savedPPFinish);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", "PP Finish ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (!(savedPreinfusion < 0) || savedPreinfusion != NULL ) {
    EEPROM.put(EEP_PREINFUSION, savedPreinfusion);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", "PREINFUSION ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (!(savedPProfile < 0) || savedPProfile != NULL ) {
    EEPROM.put(EEP_P_PROFILE, savedPProfile);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", "P-PROFILE ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (!(savedPreinfSec < 0) || savedPreinfSec != NULL ) {
    EEPROM.put(EEP_PREINFUSION_SEC, savedPreinfSec);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", "PREINFUSION SEC ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (!(savedPreinfBar < 0) || savedPreinfBar != NULL ) {
    EEPROM.put(EEP_PREINFUSION_BAR, savedPreinfBar);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", "PREINFUSION BAR ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedRegVolt > 0 && savedRegHz > 0 ) {
    EEPROM.put(EEP_REGPWR_V, savedRegVolt);
    EEPROM.put(EEP_REGPWR_HZ, savedRegHz);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", "REG PWR ERROR!");
    myNex.writeStr("page popupMSG");
    delay(5);
  }

  if (allValuesUpdated == 12) {
    myNex.writeStr("popupMSG.t0.txt","UPDATE SUCCESSFUL!");
    myNex.writeStr("page popupMSG");
  }
}

//#############################################################################################
//###############################_____HELPER_FUCTIONS____######################################
//#############################################################################################

//Function to get the state of the brew switch button
//returns true or false based on the read P(power) value
bool brewState() {
  float P;
  //Monitors the current flowing through the ACS712 circuit
  if (regionVolts > 200) {
    P = regionVolts * sensor.getCurrentAC();
    if ( P >= 45 ) return true;
    else return false;
  }else if (regionVolts < 200) {
    P = regionVolts * sensor.getCurrentAC(regionHz);
    if ( P >= 45 ) return true;
    else return false;
  }
}


bool brewTimer(bool c) {
  if (myNex.currentPageId == 0 && c == 1) {  
    if (brewState() == true) {
      myNex.writeNum("page0.timer.en", 1);
    }
  }else if(myNex.currentPageId == 0 && c == 0) { 
    myNex.writeNum("page0.timer.en", 0);
  }
}

//#############################################################################################
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

void deScale(bool c) {
  static bool blink = true;
  static unsigned long timer = millis();
  static uint8_t currentCycleRead = myNex.readNumber("j0.val");
  static uint8_t lastCycleRead = 10;
  static bool descaleFinished = false;
  if (brewState() == true && descaleFinished == false) {
    if (currentCycleRead < lastCycleRead) { // descale in cycles for 5 times then wait according to the below condition
      if (blink == true) { // Logic that switches between modes depending on the $blink value
        dimmer.setPower(dimmerDescaleMaxValue);
        if (millis() - timer > DESCALE_PHASE1_EVERY) { //set dimmer power to max descale value for 10 sec
          if (currentCycleRead >=100) descaleFinished = true;
          blink = false;
          currentCycleRead = myNex.readNumber("j0.val");
          timer = millis();
        }
      }else {
        dimmer.setPower(dimmerDescaleMinValue);
        if (millis() - timer > DESCALE_PHASE2_EVERY) { //set dimmer power to min descale value for 20 sec
          blink = true;
          currentCycleRead++;
          if (currentCycleRead<100) myNex.writeNum("j0.val", currentCycleRead);
          timer = millis();
        }
      }
    }else {
      dimmer.setPower(dimmerDescaleMinValue);
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
  }else if (brewState() == true && descaleFinished == true){
    dimmer.setPower(dimmerDescaleMinValue);
    if ((millis() - timer) > 1000) {
      myNex.writeStr("t14.txt", "FINISHED!");
      timer=millis();
    }
  }else{
    currentCycleRead = 0;
    lastCycleRead = 10;
    descaleFinished = false;
    timer = millis();
  }
  heatCtrl(); //keeping it at temp
}


//#############################################################################################
//###############################____PRESSURE_CONTROL____######################################
//#############################################################################################


// Pressure profiling function, uses dimmer to dim the pump 
// as time passes, starts dimming at about 15 seconds mark 
// goes from 9bar to the lower threshold set in settings(default 4bar)
void autoPressureProfile() {
  static bool setPerformed = 0, phase_1 = 1, phase_2 = 0, updateTimer = 1;
  static unsigned long timer = millis();
  static uint8_t dimmerOutput;
  static uint8_t dimmerNewPowerVal;

  if (brewState() == true) { //runs this only when brew button activated and pressure profile selected  
    if (updateTimer == 1) {
      timer = millis();
      updateTimer = 0;
    }
    if (phase_1 == true) { //enters phase 1
      if ((millis() - timer)>8000) { // the actions of this if block are run after 15 seconds have passed since starting brewing
        phase_1 = 0;
        phase_2 = 1;
        timer = millis();
      }
      brewTimer(1);
      dimmer.setPower(ppressureProfileStartBar);
    } else if (phase_2 == true) { //enters pahse 2
      if (millis() - timer > DIMMER_UPDATE_EVERY) { // runs the below block every half second
        if (ppressureProfileStartBar > ppressureProfileFinishBar) {
          dimmerOutput+=round(ppressureProfileStartBar/ppressureProfileFinishBar)*2;
          dimmerNewPowerVal=ppressureProfileStartBar-dimmerOutput; //calculates a new dimmer power value every second given the max and min
          if (dimmerNewPowerVal<ppressureProfileFinishBar) dimmerNewPowerVal=ppressureProfileFinishBar;  // limits range of sensor values to between ppressureProfileStartBar and ppressureProfileFinishBar
          dimmer.setPower(dimmerNewPowerVal);
        }else if (ppressureProfileStartBar < ppressureProfileFinishBar) {
          dimmerOutput+=round(ppressureProfileFinishBar/ppressureProfileStartBar)*2;
          dimmerNewPowerVal = map(dimmerOutput, 0, 100, ppressureProfileStartBar, ppressureProfileFinishBar); //calculates a new dimmer power value every second given the max and min
          if (dimmerNewPowerVal>ppressureProfileFinishBar) dimmerNewPowerVal=ppressureProfileFinishBar;  // limits range of sensor values to between ppressureProfileStartBar and ppressureProfileFinishBar
          dimmer.setPower(dimmerNewPowerVal);
        }
        timer = millis();
      } 
    }
  }else { 
    brewTimer(0);
    if (selectedOperationalMode == 1 ) dimmer.setPower(ppressureProfileStartBar);
    if (selectedOperationalMode == 4 ) preinfusionFinished = false;
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
  volatile uint8_t power_reading = myNex.readNumber("page0.h0.val");
  if (brewState()==true) {
    brewTimer(1);
    dimmer.setPower(power_reading);
  }else {
    brewTimer(0);
  }
  heatCtrl();
}
//#############################################################################################
//###############################____PREINFUSION_CONTROL____###################################
//#############################################################################################

// Pump dimming during brew for preinfusion
void preInfusion(bool c) {
  static bool blink = true;
  static bool exitPreinfusion;
  static unsigned long timer = millis();

  if (brewState() == true) {
    if (exitPreinfusion == false) { //main preinfusion body
      if (blink == true) { // Logic that switches between modes depending on the $blink value
        brewTimer(1);
        dimmer.setPower(preinfuseBar);
        if ((millis() - timer) > (preinfuseTime*1000)) {
          brewTimer(0);
          // delay(50);
          blink = false;
          timer = millis();
        }
      }else {
        brewTimer(0);
        dimmer.setPower(dimmerMinPowerValue);
        if (millis() - timer > PI_SOAK_FOR) { 
          exitPreinfusion = true;
          blink = true;
          timer = millis();
        }
      }
    }else if(exitPreinfusion == true && selectedOperationalMode == 1){ // just pre-infusion
      brewTimer(1);
      dimmer.setPower(dimmerMaxPowerValue);
    }else if(exitPreinfusion == true && selectedOperationalMode == 4){ // pre-infusion with pressure profiling on
      brewTimer(0);
      preinfusionFinished = true;
      dimmer.setPower(ppressureProfileStartBar);
    }
  }else { //resetting all the values
    brewTimer(0);
    dimmer.setPower(preinfuseBar);
    exitPreinfusion = false;
    timer = millis();
  }
  heatCtrl(); //keeping it at temp
}
