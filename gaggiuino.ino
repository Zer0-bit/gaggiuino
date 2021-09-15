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
#define GET_KTYPE_READ_EVERY 350 // thermocouple data read interval not recommended to be changed to lower than 250
#define REFRESH_SCREEN_EVERY 350 // Screen refresh interval
#define MAX_SETPOINT_VALUE 110 //Defines the max value of the setpoint
#define dimmerMinPowerValue 30
#define dimmerMaxPowerValue 97
#define dimmerDescaleMinValue 40
#define dimmerDescaleMaxValue 50
#define dimmerPreinfusionValue 50

//RAM debug
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;


//Init the thermocouple with the appropriate pins defined above with the prefix "thermo"
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
// EasyNextion object init
EasyNex myNex(Serial);
//Init the ACS712 hall sensor
ACS712 sensor(ACS712_20A, brewSwitchPin);
// RobotDYN Dimmer object init
dimmerLamp dimmer(dimmerPin); //initialase port for dimmer for MEGA, Leonardo, UNO, Arduino M0, Arduino Zero




// Some vars are better global
float currentTempReadValue = 0.0;
float lastReadTempValue = 0.0;
bool POWER_ON;
bool  descaleCheckBox = 0;
bool  preinfusionCheckBox = 0;
bool  pressureProfileCheckBox = 0;
bool  valuesRefreshOnPageChange = 0;
bool preinfusionFinished;
uint16_t  HPWR;
uint16_t  HPWR_OUT;
uint16_t  setPoint;
uint16_t  offsetTemp;
uint8_t  MainCycleDivider;
uint8_t  BrewCycleDivider;
uint8_t  preinfuseTime;
uint8_t preinfuseBar;
uint8_t ppressureProfileStartBar;
uint8_t ppressureProfileFinishBar;
uint8_t selectedOperationalMode;
unsigned long thermoTimer = millis();

// Declaring local vars
// EEPROM  stuff
uint16_t  EEP_SETPOINT = 1;
uint16_t  EEP_OFFSET = 20;
uint16_t  EEP_HPWR = 40;
uint16_t  EEP_M_DIVIDER = 60;
uint16_t  EEP_B_DIVIDER = 80;
uint16_t  EEP_P_START = 100;
uint16_t  EEP_P_FINISH = 120;
uint16_t  EEP_P_AUTO = 140;
uint16_t  EEP_P_MANUAL = 160;


void setup() {
  
  Serial.begin(115200); // switching our board to the new serial speed

  // To debug correct work of the bellow feature later
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
  if (EEPROM.read(0) != 252 || EEPROM.read(EEP_SETPOINT) == NULL || EEPROM.read(EEP_SETPOINT) == 65535) {
    Serial.println("SECU_CHECK FAILED! Applying defaults!");
    EEPROM.put(0, 252);
    EEPROM.put(EEP_SETPOINT, 101);
    EEPROM.put(EEP_OFFSET, 8);
    EEPROM.put(EEP_HPWR, 550);
    EEPROM.put(EEP_M_DIVIDER, 5);
    EEPROM.put(EEP_B_DIVIDER, 2);
    EEPROM.put(EEP_P_START, 97);
    EEPROM.put(EEP_P_FINISH, 30);
    EEPROM.put(EEP_P_AUTO, 0);
    EEPROM.put(EEP_P_MANUAL, 0);
  }
  // Applying our saved EEPROM saved values
  uint16_t init_val;
  // Making sure we're getting the right values and sending them to the LCD module

  EEPROM.get(EEP_SETPOINT, init_val);
  if ( init_val > 0 && init_val != NULL ) myNex.writeNum("page1.n0.val", init_val);

  EEPROM.get(EEP_OFFSET, init_val);
  if ( init_val > 0 && init_val != NULL ) myNex.writeNum("page1.n1.val", init_val);

  EEPROM.get(EEP_HPWR, init_val);
  if (  init_val > 0 && init_val != NULL ) myNex.writeNum("page1.n2.val", init_val);

  EEPROM.get(EEP_M_DIVIDER, init_val);
  if ( init_val > 1 && init_val != NULL ) myNex.writeNum("page1.n4.val", init_val);

  EEPROM.get(EEP_B_DIVIDER, init_val);
  if (  init_val > 1 && init_val != NULL ) myNex.writeNum("page1.n3.val", init_val);

  EEPROM.get(EEP_P_START, init_val);
  if (  init_val > 1 && init_val != NULL ) myNex.writeNum("page2.n0.val", init_val);

  EEPROM.get(EEP_P_FINISH, init_val);
  if (  init_val > 1 && init_val != NULL ) myNex.writeNum("page2.n1.val", init_val);

  EEPROM.get(EEP_P_AUTO, init_val);
  if (  !(init_val < 0) && init_val < 2 && init_val != NULL ) myNex.writeNum("page2.c3.val", init_val);

  EEPROM.get(EEP_P_MANUAL, init_val);
  if (  !(init_val < 0) && init_val < 2 && init_val != NULL ) myNex.writeNum("page2.c4.val", init_val);

 myNex.writeStr("page 0");
 delay(10);
 myNex.lastCurrentPageId = 1;
 delay(5);
 POWER_ON = true;
}

//##############################################################################################################################
//############################################________________MAIN______________################################################
//##############################################################################################################################


//Main loop where all the bellow logic is continuously run
void loop() {
  Power_ON_Values_Refresh();
  myNex.NextionListen();
  kThermoRead();
  modeSelect();
  screenRefresh();
  pageValuesRefresh();
  // brewCycleTracker();
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

void Power_ON_Values_Refresh() {  // Refreshing our values on first start and subsequent page changes

  if (POWER_ON == true) {
    // Making sure the serial communication finishes sending all the values
    setPoint = myNex.readNumber("page1.n0.val");  // reading the setPoint value from the lcd
    if ( setPoint == NULL || setPoint <= 0 || setPoint > MAX_SETPOINT_VALUE ) setPoint = myNex.readNumber("page1.n0.val");

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

    selectedOperationalMode = myNex.readNumber("page2.mode_select.val");
    if (selectedOperationalMode < 0 || selectedOperationalMode > 7) selectedOperationalMode = myNex.readNumber("page2.mode_select.val");

    preinfuseTime = myNex.readNumber("page2.n3.val");
    if (preinfuseTime < 0 || preinfuseTime > 10) preinfuseTime = myNex.readNumber("page2.n3.val");

    preinfuseBar = myNex.readNumber("page2.preinf_pwr.val");
    if (preinfuseBar < 0 || preinfuseBar > 97) preinfuseBar = myNex.readNumber("page2.preinf_pwr.val");
    
    ppressureProfileStartBar = myNex.readNumber("page2.pps_var.val");
    if (ppressureProfileStartBar < 0 || ppressureProfileStartBar > 97) ppressureProfileStartBar = myNex.readNumber("page2.pps_var.val");
    ppressureProfileFinishBar = myNex.readNumber("page2.ppf_var.val");
    if (ppressureProfileFinishBar < 0 || ppressureProfileFinishBar > 97) ppressureProfileFinishBar = myNex.readNumber("page2.ppf_var.val");

    myNex.lastCurrentPageId = myNex.currentPageId;
    POWER_ON = false;
  }
}

void pageValuesRefresh() {  // Refreshing our values on page changes

  // myNex.currentPageId = myNex.readNumber("dp"); //reading the value of the currently loaded page

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

    selectedOperationalMode = myNex.readNumber("page2.mode_select.val");
    if (selectedOperationalMode < 0 || selectedOperationalMode > 10) selectedOperationalMode = myNex.readNumber("page2.mode_select.val");

    preinfuseTime = myNex.readNumber("page2.n3.val");
    if (preinfuseTime < 0 || preinfuseTime > 10) preinfuseTime = myNex.readNumber("page2.n3.val");

    preinfuseTime = myNex.readNumber("page2.n3.val");
    if (preinfuseTime < 0 || preinfuseTime > 10) preinfuseTime = myNex.readNumber("page2.n3.val");

    preinfuseBar = myNex.readNumber("page2.preinf_pwr.val");
    if (preinfuseBar < 0 || preinfuseBar > 97) preinfuseBar = myNex.readNumber("page2.preinf_pwr.val");
    
    ppressureProfileStartBar = myNex.readNumber("page2.pps_var.val");
    if (ppressureProfileStartBar < 0 || ppressureProfileStartBar > 97) ppressureProfileStartBar = myNex.readNumber("page2.pps_var.val");
    ppressureProfileFinishBar = myNex.readNumber("page2.ppf_var.val");
    if (ppressureProfileFinishBar < 0 || ppressureProfileFinishBar > 97) ppressureProfileFinishBar = myNex.readNumber("page2.ppf_var.val");


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
// The temperature, preinfusion, dimming, LCD update, etc control logic is all in the bellow functions
void justDoCoffee() {
  uint8_t HPWR_LOW= HPWR/MainCycleDivider;
  // Calculating the boiler heating power range based on the bellow input values
  HPWR_OUT = map(currentTempReadValue, setPoint - 10, setPoint, HPWR, HPWR_LOW);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, HPWR);  // limits range of sensor values to between 110 and 550

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
    } else if (currentTempReadValue >= (float)setPoint - 3.00 && currentTempReadValue <= float(setPoint - 1.00)) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
    } else if (currentTempReadValue >= (float)setPoint - 1.00 && currentTempReadValue < (float)setPoint - 0.50) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT/2);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
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
  // Calculating the boiler heating power range based on the bellow input values
  HPWR_OUT = map(currentTempReadValue, setPoint - 10, setPoint, HPWR, HPWR_LOW);
  HPWR_OUT = constrain(HPWR_OUT, HPWR_LOW, HPWR);  // limits range of sensor values to between 110 and 550

  if (brewState() == true) {
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
    } else if (currentTempReadValue >= (float)setPoint - 3.00 && currentTempReadValue <= float(setPoint - 1.00)) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
    } else if (currentTempReadValue >= (float)setPoint - 1.00 && currentTempReadValue < (float)setPoint - 0.50) {
      PORTB |= _BV(PB0);   // relayPin -> HIGH
      delay(HPWR_OUT/2);  // delaying the relayPin state for <HPWR_OUT> ammount of time
      PORTB &= ~_BV(PB0);  // relayPin -> LOW
      delay(HPWR_OUT);  // delaying the relayPin state for <HPWR_OUT> ammount of time
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
    // myNex.writeNum("page0.n1.val", offsetTemp);
    // float P = 230 * sensor.getCurrentAC()*10;
    // myNex.writeNum("page0.n2.val", currentTempReadValue);
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
  uint8_t savedAutoPP = myNex.readNumber("page2.c3.val");
  uint8_t savedManPP = myNex.readNumber("page2.c4.val");
  uint8_t allValuesUpdated = 0;

  if (EEP_SETPOINT >= 19 ) EEP_SETPOINT = 1;
  if (EEP_OFFSET >= 39 ) EEP_OFFSET = 20;
  if (EEP_HPWR >= 59 ) EEP_HPWR = 40;
  if (EEP_M_DIVIDER >= 79 ) EEP_M_DIVIDER = 60;
  if (EEP_B_DIVIDER >= 99 ) EEP_B_DIVIDER = 80;
  if (EEP_P_START >= 119) EEP_P_START = 100;
  if (EEP_P_FINISH >= 139) EEP_P_FINISH = 120;
  if (EEP_P_AUTO >= 159) EEP_P_AUTO = 140;
  if (EEP_P_MANUAL >= 179) EEP_P_MANUAL = 160;

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
    myNex.writeStr("popupMSG.t0.txt", (" OFFSET ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedHPWR != NULL && savedHPWR > 0) {
    EEPROM.put(EEP_HPWR, savedHPWR);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", ("HPWR ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedMainCycleDivider != NULL && savedMainCycleDivider >= 1) {
    EEPROM.put(EEP_M_DIVIDER, savedMainCycleDivider);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", (" M_DIV ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedBrewCycleDivider != NULL && savedBrewCycleDivider >= 1) {
    EEPROM.put(EEP_B_DIVIDER, savedBrewCycleDivider);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", (" B_DIV ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedPPStart != NULL && savedPPStart >= 1) {
    EEPROM.put(EEP_P_START, savedPPStart);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", ("PP Start ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (savedPPFinish != NULL && savedPPFinish >= 1) {
    EEPROM.put(EEP_P_FINISH, savedPPFinish);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", ("PP Finish ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if (!(savedAutoPP < 0) || savedAutoPP != NULL ) {
    EEPROM.put(EEP_P_AUTO, savedAutoPP);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", ("AUTO PP ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
  if ( !(savedManPP < 0) || savedManPP != NULL ) {
    EEPROM.put(EEP_P_MANUAL, savedManPP);
    allValuesUpdated++;
  }else {
    myNex.writeStr("popupMSG.t0.txt", ("MANUAL PP ERROR!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }

  if (allValuesUpdated == 9) {
    myNex.writeStr("popupMSG.t0.txt", ("UPDATE SUCCESSFUL!"));
    myNex.writeStr("page popupMSG");
    delay(5);
  }
}

//#############################################################################################
//###############################_____HELPER_FEATURES____######################################
//#############################################################################################

//Function to get the state of the brew switch button
//returns true or false based on the read P(power) value
bool brewState() {
  //Monitors the current flowing through the ACS712 circuit
  float P = 230 * sensor.getCurrentAC();
  //When it exceeds the set limit 
  if ( P > 25 ) return true;
  else return false;
}


bool brewTimer(bool c) {
  if (myNex.currentPageId == 0 && c == 1) {  
    if (brewState() == true) {
      myNex.writeNum("page0.tim_v.val", 1);
    }
  }else if(myNex.currentPageId == 0 && c == 0) { 
    myNex.writeNum("page0.tim_v.val", 0);
  }
}

//#############################################################################################
//###############################____DESCALE__CONTROL____######################################
//#############################################################################################

void deScale(bool c) {
  static bool blink = true;
  static unsigned long timer = millis();
  static int i = 0;
  if (c == true) {
    if (i < 6) { // descale in cycles for 5 times then wait according to the bellow condition
      if (blink == true) { // Logic that switches between modes depending on the $blink value
        dimmer.setPower(dimmerDescaleMaxValue);
        if (millis() - timer > 10000) { //set dimmer power to max descale value for 10 sec
          blink = false;
          timer = millis();
        }
      }else {
        dimmer.setPower(dimmerDescaleMinValue);
        if (millis() - timer > 20000) { //set dimmer power to min descale value for 20 sec
          blink = true;
          i++;
          timer = millis();
        }
      }
    }else {
      dimmer.setPower(30);
      if ((millis() - timer) > 300000) { //nothing for 5 minutes
        i=0;
        timer = millis();
      } 
    }
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
  static bool setPerformed = 0, phase_1 = 1, phase_2 = 0;
  static unsigned long timer = millis();
  static uint8_t dimmerOutput;
  static uint8_t dimmerNewPowerVal;
  
  // bool tmp = myNex.readNumber("page2.c2.val");
  //DEBUG READ VALUES
  // myNex.writeNum("page0.n2.val", ppressureProfileStartBar);
  //END DEBUG 

  if (brewState() == true ) { //runs this only when brew button activated and pressure profile selected  
    brewTimer(1);
    if (phase_1 == true) { //enters phase 1
      if ((millis() - timer)>8000) { // the actions of this if block are run after 15 seconds have passed since starting brewing
        phase_1 = 0;
        phase_2 = 1;
        timer = millis();
      }
      dimmer.setPower(ppressureProfileStartBar);
      myNex.writeNum("page0.n2.val",ppressureProfileStartBar);
    } else if (phase_2 == true) { //enters pahse 2
      if (millis() - timer > 500) { // runs the bellow block every second
        if (ppressureProfileStartBar > ppressureProfileFinishBar) {
          dimmerOutput+=ppressureProfileStartBar/ppressureProfileFinishBar*2;
          dimmerNewPowerVal=ppressureProfileStartBar-dimmerOutput; //calculates a new dimmer power value every second given the max and min
          if (dimmerNewPowerVal<ppressureProfileFinishBar) dimmerNewPowerVal=ppressureProfileFinishBar;  // limits range of sensor values to between ppressureProfileStartBar and ppressureProfileFinishBar
          dimmer.setPower(dimmerNewPowerVal);
          myNex.writeNum("page0.n2.val",dimmerNewPowerVal);
        }else if (ppressureProfileStartBar < ppressureProfileFinishBar) {
          dimmerOutput+=ppressureProfileFinishBar/ppressureProfileStartBar*2;
          dimmerNewPowerVal = map(dimmerOutput, 0, 100, ppressureProfileStartBar, ppressureProfileFinishBar); //calculates a new dimmer power value every second given the max and min
          if (dimmerNewPowerVal>ppressureProfileFinishBar) dimmerNewPowerVal=ppressureProfileFinishBar;  // limits range of sensor values to between ppressureProfileStartBar and ppressureProfileFinishBar
          // dimmerNewPowerVal = constrain(dimmerNewPowerVal, ppressureProfileStartBar, ppressureProfileFinishBar);  // limits range of sensor values to between ppressureProfileStartBar and ppressureProfileFinishBar
          dimmer.setPower(dimmerNewPowerVal);
          myNex.writeNum("page0.n2.val",dimmerNewPowerVal);
        }
        timer = millis();
      } 
    }
  }else { //Manual preinfusion control
    brewTimer(0);
    dimmer.setPower(ppressureProfileStartBar);
    timer = millis();
    phase_2 = false;
    phase_1=true;
    dimmerOutput=0;
    dimmerNewPowerVal=0;
  }
  heatCtrl(); // Keep that water at temp
}

void manualPressureProfile() {
  uint8_t power_reading = myNex.readNumber("page0.p_var.val");
  if (brewState()==true) {
    brewTimer(1);
    dimmer.setPower(power_reading);
  }else {
    brewTimer(0);
  }
}
//#############################################################################################
//###############################____PREINFUSION_CONTROL____###################################
//#############################################################################################

// Pump dimming during brew for preinfusion
void preInfusion(bool c) {
  static bool blink = true;
  static bool exitPreinfusion;
  static unsigned long timer = millis();

  if (brewState() == true && c == true) {
    if (exitPreinfusion == false) {
      if (blink == true) { // Logic that switches between modes depending on the $blink value
        brewTimer(1);
        dimmer.setPower(preinfuseBar);
        if ((millis() - timer) > (preinfuseTime*1000)) {
          blink = false;
          timer = millis();
        }
      }else {
        brewTimer(0);
        dimmer.setPower(dimmerMinPowerValue);
        if (millis() - timer > 3000) { 
          exitPreinfusion = true;
          blink = true;
          timer = millis();
        }
      }
    }else if(exitPreinfusion == true && pressureProfileCheckBox == false){
      brewTimer(1);
      dimmer.setPower(dimmerMaxPowerValue);
    }else {
      preinfusionFinished = true;
    } 
  }else if(brewState() == false && c == true) {
    brewTimer(0);
    exitPreinfusion = false;
    timer = millis();
    preinfusionFinished = false;
  }
  heatCtrl(); //keeping it at temp
}


uint16_t getFreeSram() {
  uint8_t newVariable;
  // heap is empty, use bss as start memory address
  if ((uint16_t)__brkval == 0)
    return (((uint16_t)&newVariable) - ((uint16_t)&__bss_end));
  // use heap end as the start of the memory address
  else
    return (((uint16_t)&newVariable) - ((uint16_t)__brkval));
}