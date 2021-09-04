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
#define dimmerMinValue 90
#define dimmerMaxValue 97
#define dimmerDescaleMinValue 1
#define dimmerDescaleMaxValue 5

//RAM debug
// extern unsigned int __bss_end;
// extern unsigned int __heap_start;
// extern void *__brkval;


//Init the thermocouple with the appropriate pins defined above with the prefix "thermo"
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
// EasyNextion object init
EasyNex myNex(Serial);
//Init the ACS712 hall sensor
ACS712 sensor(ACS712_20A, brewSwitchPin);
// RobotDYN Dimmer object init
dimmerLamp dimmer(dimmerPin); //initialase port for dimmer for MEGA, Leonardo, UNO, Arduino M0, Arduino Zero


// Global var  - just defined globally to avoid reading the temp sensor in every function
float currentTempReadValue = 0.00;
bool descaleEnabled = 0, powerBackOnDescaleFinish = 0, preinfusionCheckBox = 0;
byte setPoint = 0;
byte offsetTemp = 0;
uint16_t brewTimeDelayTemp = 0;
byte brewTimeDelayDivider = 0;
char waterTempPrint[6];
unsigned long timer = millis(), timer_lcd = millis();
bool lcdValuesUpdated = false;

// EEPROM  stuff - alo global cause used in multiple functions
int EEP_ADDR1 = 1, EEP_ADDR2 = 20, EEP_ADDR3 = 40, EEP_ADDR4 = 60;


void setup() {
  //myNex.begin(115200); //this has been set as an init baud parameter on the Nextion LCD
  Serial.begin(115200); // switching our board to the new serial speed

  // To debug correct work of the bellow feature later
  dimmer.begin(TOGGLE_MODE, OFF); //dimmer initialisation: name.begin(MODE, STATE)
  dimmer.toggleSettings(dimmerMinValue, dimmerMaxValue); //Name.toggleSettings(MIN, MAX);
  dimmer.setState(ON); // state: dimmer1.setState(ON/OFF);

  // Calibrating the hall current sensor
  sensor.calibrate();

  // relay port init and set initial operating mode
  pinMode(relayPin, OUTPUT);
  pinMode(brewSwitchPin, INPUT);

  // Chip side  HIGH/LOW  specification
  PORTB &= ~_BV(PB0);  // relayPin LOW

  // Will wait hereuntil full serial is established, this is done so the LCD fully initializes before passing the EEPROM values
  delay(500);
  // Applying the EEPROM saved values
  uint8_t tmp1 = 0;
  uint8_t tmp2 = 0;
  uint16_t tmp3 = 0;
  uint8_t tmp4 = 0;

  // Making sure we're getting the right values and sending them to the display
  tmp1 = readIntFromEEPROM(EEP_ADDR1);
  if ( !(tmp1<0)  && tmp1 != NAN) myNex.writeNum("page1.n0.val", tmp1);

  tmp2 = readIntFromEEPROM(EEP_ADDR2);
  if ( !(tmp2<0)  && tmp2 != NAN) myNex.writeNum("page1.n1.val", tmp2);

  tmp3 = readIntFromEEPROM(EEP_ADDR3);
  if ( !(tmp3<0)  && tmp3 != NAN) myNex.writeNum("page2.n0.val", tmp3);

  tmp4 = readIntFromEEPROM(EEP_ADDR4);
  if ( !(tmp4<0)  && tmp4 != NAN) myNex.writeNum("page2.n1.val", tmp4);
}


//Main loop where all the bellow logic is continuously run
void loop() {
  myNex.NextionListen();
  // Reading the temperature every 350ms between the loops
  if ((millis() - timer) > 350UL){
    currentTempReadValue = thermocouple.readCelsius();
    if (currentTempReadValue == NAN || currentTempReadValue < 0) currentTempReadValue = thermocouple.readCelsius();  // Making sure we're getting a value
    timer += 350UL;
  }
  if (brewState() == true) {

  }
  doCoffee();
  // Descale function
  if (descaleEnabled == true) {
    deScale(descaleEnabled);
  }else {
    if (powerBackOnDescaleFinish == true) {
      dimmer.toggleSettings(dimmerMinValue, dimmerMaxValue); //Name.toggleSettings(MIN, MAX);
      dimmer.setPower(dimmerMaxValue);
      powerBackOnDescaleFinish = false;
    }
  }
}


//  ALL used functions declared bellow
// The temp control logic
void doCoffee() {
  // Getting the latest LCD side set temp settings 
  if ( lcdValuesUpdated == false ) {
    // Making sure the serial communication finishes sending all the values
    setPoint = myNex.readNumber("page1.n0.val");  // reading the setPoint value from the lcd
    if (!(setPoint<0) && setPoint != NAN && setPoint > 85);
    else setPoint = myNex.readNumber("page1.n0.val");

    offsetTemp = myNex.readNumber("page1.n1.val");  // reading the offset value from the lcd
    if ( !(offsetTemp<0) && offsetTemp != NAN );
    else offsetTemp = myNex.readNumber("page1.n1.val");

    brewTimeDelayTemp = myNex.readNumber("page2.n0.val");  // reading the brew time delay used to apply heating in waves
    if ( !(brewTimeDelayTemp<0) && brewTimeDelayTemp != NAN );
    else brewTimeDelayTemp = myNex.readNumber("page2.n0.val");

    brewTimeDelayDivider = myNex.readNumber("page2.n1.val");  // reading the delay divider
    if ( !(brewTimeDelayDivider<0) && brewTimeDelayDivider != NAN );
    else brewTimeDelayDivider = myNex.readNumber("page2.n1.val");

    // reding the descale value which should be 0 or 1
    descaleEnabled = myNex.readNumber("page2.c1.val");
    if (descaleEnabled < 0 || descaleEnabled > 1 || descaleEnabled == NAN) descaleEnabled = myNex.readNumber("page2.c1.val");

    // reding the preinfusion value which should be 0 or 1
    preinfusionCheckBox = myNex.readNumber("page2.c0.val");
    if (preinfusionCheckBox < 0 || preinfusionCheckBox > 1 || preinfusionCheckBox == NAN) preinfusionCheckBox = myNex.readNumber("page2.c0.val");

    // some boolean logic controling values setting
    powerBackOnDescaleFinish = true; // setting this to true so we make sure the pump has power after we turn off the descale mode
    lcdValuesUpdated == true; // setting this to TRUE so the condition is skipped on subsequent iterations
  }

  // Getting the boiler heating power range
  uint16_t powerOutput = map(currentTempReadValue, setPoint - 10, setPoint, brewTimeDelayTemp, brewTimeDelayTemp / brewTimeDelayDivider);
  float tmp1;
  if (powerOutput > brewTimeDelayTemp) {
    powerOutput = brewTimeDelayTemp;
  } else if (powerOutput < brewTimeDelayTemp / brewTimeDelayDivider) {
    powerOutput = brewTimeDelayTemp / brewTimeDelayDivider;
  }


  // Applying the powerOutput variable as part of the relay switching logic
  if (brewState() == true) {
    if (currentTempReadValue < setPoint+0.5) {
      PORTB |= _BV(PB0);   // relayPIN -> HIGH
      delay(powerOutput/2);  // delaying the relayPin state for <powerOutput> ammount of time
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
      delay(powerOutput); 
    }
  } else {
    if (currentTempReadValue < (float)setPoint - 10.00 && !(currentTempReadValue < 0.00) && currentTempReadValue != NAN) {
      PORTB |= _BV(PB0);  // relayPIN -> HIGH
    } else if (currentTempReadValue >= (float)setPoint - 10.00 && currentTempReadValue < (float)setPoint - 3.00) {
      PORTB |= _BV(PB0);   // relayPIN -> HIGH
      delay(powerOutput);  // delaying the relayPin state for <powerOutput> ammount of time
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
    } else if (currentTempReadValue >= (float)setPoint - 3.00 && currentTempReadValue <= float(setPoint - 1.00)) {
      PORTB |= _BV(PB0);   // relayPIN -> HIGH
      delay(powerOutput);  // delaying the relayPin state for <powerOutput> ammount of time
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
      delay(powerOutput);  // delaying the relayPin state for <powerOutput> ammount of time
    } else if (currentTempReadValue >= (float)setPoint - 1.00 && currentTempReadValue < (float)setPoint - 0.50) {
      PORTB |= _BV(PB0);   // relayPIN -> HIGH
      delay(powerOutput/2);  // delaying the relayPin state for <powerOutput> ammount of time
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
      delay(powerOutput);  // delaying the relayPin state for <powerOutput> ammount of time
    } else {
      PORTB &= ~_BV(PB0);  // relayPIN -> LOW
    }
  }

  // Updating the LCD every 500ms
  if (myNex.currentPageId == 0) {
    //Declaring some local variables we'll use to control various features displayed on the LCD 
    static bool blink = true, steam_reset = false;
    static unsigned long timer_s1 = millis(), timer_s2 = millis();
    unsigned long currentMillis = millis();
    if ((millis() - timer_lcd) > 500UL){
      if (currentTempReadValue < 115.00 ) {
        if (steam_reset == true) {
          myNex.writeStr("vis t1,0"); //Resetting the "STEAMING!" message
          steam_reset = false;
        }
        if (powerOutput > brewTimeDelayTemp) {
          powerOutput = brewTimeDelayTemp;
        } else if (powerOutput < brewTimeDelayTemp / brewTimeDelayDivider) {
          powerOutput = brewTimeDelayTemp / brewTimeDelayDivider;
        } else {
          myNex.writeNum("page0.n0.val", powerOutput);
        }
        myNex.writeNum("page0.n0.val", powerOutput);
      } else if (currentTempReadValue > 115.00 && !(currentTempReadValue < 0) && currentTempReadValue != NAN ) {
        if (blink == true) {
          if (currentMillis >= timer_s1 + 1000UL) {
            timer_s1 += 1000UL;
            blink = false;
            steam_reset = true;
          }
          myNex.writeStr("vis t1,1");
          myNex.writeNum("page0.n0.val", NAN);
          tmp1 = currentTempReadValue - (float)offsetTemp;
          dtostrf(tmp1, 6, 2, waterTempPrint);
          myNex.writeStr("t0.txt", waterTempPrint);
        } else {
          myNex.writeStr("vis t1,0");
          myNex.writeNum("page0.n0.val", NAN);
          tmp1 = currentTempReadValue - (float)offsetTemp;
          dtostrf(tmp1, 6, 2, waterTempPrint);
          myNex.writeStr("page0.t0.txt", waterTempPrint);
          if (currentMillis >= timer_s2 + 1000UL) {
            timer_s2 += 1000UL;
            blink = true;
            steam_reset = true;
          }
        }
      } else {
        myNex.writeNum("page0.n0.val", NAN);
      }
      tmp1 = currentTempReadValue - (float)offsetTemp;
      dtostrf(tmp1, 6, 2, waterTempPrint);
      myNex.writeStr("page0.t0.txt", waterTempPrint);  // Printing the current water temp values to the display
      myNex.writeNum("page0.n1.val", getFreeSram());
      timer_lcd += 500UL;
    }
  } 
}

// EEPROM WRITE
void writeIntIntoEEPROM(int address, int number) {
  EEPROM.write(address + 1, number & 0xFF);
  EEPROM.write(address, number >> 8);
}
//EEPROM READ
int readIntFromEEPROM(int address) {
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

// Save the desired temp values to EEPROM
void trigger1() {
  int savedBoilerTemp = myNex.readNumber("page1.n0.val");
  int savedOffsetTemp = myNex.readNumber("page1.n1.val");
  int savedBrewTimeDelay = myNex.readNumber("page2.n0.val");
  int savedbrewTimeDivider = myNex.readNumber("page2.n1.val");

  if (EEP_ADDR1 >= 19) EEP_ADDR1 = 1;
  if (EEP_ADDR2 >= 39) EEP_ADDR1 = 20;
  if (EEP_ADDR3 >= 59) EEP_ADDR1 = 40;
  if (EEP_ADDR4 >= 79) EEP_ADDR1 = 60;

  // Reading our values from the LCD and checking whether we got proper serial communication
  if (savedBoilerTemp != 777777 && savedBoilerTemp != NAN && savedBoilerTemp > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
    myNex.writeStr("popupMSG.t0.txt", F("SUCCESS!"));
    myNex.writeStr("page popupMSG");
    delay(10);  //giving it a bit of time to finish the serial comm
  }
  if (savedOffsetTemp != 777777 && savedOffsetTemp != NAN && savedOffsetTemp > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
    myNex.writeStr("popupMSG.t0.txt", F("SUCCESS!"));
    myNex.writeStr("page popupMSG");
    delay(10);  //giving it a bit of time to finish the serial comm
  }
  if (savedBrewTimeDelay != 777777 && savedBrewTimeDelay != NAN && savedBrewTimeDelay > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
    myNex.writeStr("popupMSG.t0.txt", F("SUCCESS!"));
    myNex.writeStr("page popupMSG");
    delay(10);  //giving it a bit of time to finish the serial comm
  }
  if (savedbrewTimeDivider != 777777 && savedbrewTimeDivider != NAN && savedbrewTimeDivider > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR4, savedbrewTimeDivider);
    myNex.writeStr("popupMSG.t0.txt", F("SUCCESS!"));
    myNex.writeStr("page popupMSG");
    delay(10);  //giving it a bit of time to finish the serial comm
  }
  lcdValuesUpdated = false;
}

void trigger2() {
  lcdValuesUpdated = false;
}

// Pressure profiling function, uses dimmer to dim the pump 
// as time passes, starts dimming at about 15 seconds mark 
// goes from 9bar to the lower threshold set in settings(default 6bar)
void pressureProfile(bool c, uint8_t a) {
  static bool phase_1 = 1, phase_2 = 0, phase_3 = 0;
  static unsigned long timer = millis();
  unsigned long currentMillis = millis();
  uint16_t dimmerOutput;
  if (brewState() == true && c == true) {
    if (phase_1 == true) {
      if (currentMillis >= timer + 15000UL) {
        phase_1 = 0;
        phase_2 = 1;
      }
      dimmer.setPower(dimmerMaxValue);
    } else if (phase_2 == true) {
      if (currentMillis >= timer + 1000UL) {
        timer += 1000UL;
        // tmpDimmerVal = (uint16_t)dimmerMaxValue - a;
        dimmerOutput = map(currentMillis, timer, timer+15000UL, (uint16_t)dimmerMaxValue, (uint16_t)dimmerMaxValue - a);
        dimmer.setPower(dimmerOutput);
      } 
    }
  }
}

// Pump dimming during for preinfusion
// void preinfusion() {
//   //dealing with preinfusion here, might take more time to get it right
//  if (brewSwitchPin == HIGH && preinfusionState(preinfusionCheckBox) == true) {
//    for (unsigned long i=millis();i > millis()+5000UL;) {
//      dimmer.setPower(dimmerMaxValue/2);
//    }
//    dimmer.setPower(dimmerMaxValue);
//   //  preinfusionState(false);
//  }
// }

bool brewState() {
  if (myNex.currentPageId == 0 ) {
    float P = 230 * sensor.getCurrentAC();
    if (!(P<0.00) && P >= 100.00  && P != NAN ) return true;
    else return false;
  }
}

void deScale(bool c) {
  static bool blink = true, value_set = false, dimmerModeSet = false;
  static unsigned long timer = millis();
  static int i = 0;
  unsigned long currentMillis = millis();
  if (c == true) {
    // Switching the dimmer min and max toggle values to suit the descaling mode
    if (dimmerModeSet == false) {
      dimmer.toggleSettings(dimmerDescaleMinValue, dimmerDescaleMaxValue); //Name.toggleSettings(MIN, MAX);
      dimmerModeSet = true;
    }
    if (i < 5) { // descale in cycles for 5 times then wait according to the bellow condition
      // Logic that switches between modes depending on the $blink value
      if (blink == true) {
        if (value_set == false) { // making sure we're not constantly setting the dimmer value over serial
          dimmer.setPower(dimmerDescaleMaxValue);
          value_set = true;
        }
        if (currentMillis >= timer + 10000UL) { //set dimmer power to max descale value for 10 sec
          timer += 10000UL;
          blink = false;
          value_set = false;
        }
      } else {
        if (value_set == false) { // making sure we're not constantly setting the dimmer value over serial
          dimmer.setPower(dimmerDescaleMinValue);
          value_set = true;
        }
        if (currentMillis >= timer + 20000UL) { //set dimmer power to min descale value for 20 sec
          timer += 20000UL;
          blink = true;
          value_set = false;
          i+=1;
        }
      }
    }else {
      if (value_set == false) { // making sure we're not constantly setting the dimmer value over serial
        dimmer.setState(OFF);
        value_set = true;
      }
      if (currentMillis >= timer + 300000UL) { //do nothing for 5 minutes
        timer += 300000UL;
        dimmer.setState(ON);
        value_set = false;
        i=0;
      } 
    }
  }
  // else {
  //   dimmer.setPower(dimmerMaxValue);
  //   powerBackOnDescaleFinish = true;
  // }
}
// uint16_t getFreeSram() {
//   uint8_t newVariable;
//   // heap is empty, use bss as start memory address
//   if ((uint16_t)__brkval == 0)
//     return (((uint16_t)&newVariable) - ((uint16_t)&__bss_end));
//   // use heap end as the start of the memory address
//   else
//     return (((uint16_t)&newVariable) - ((uint16_t)__brkval));
// }
