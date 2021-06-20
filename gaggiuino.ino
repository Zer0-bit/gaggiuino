#include <RBDdimmer.h>
#include <EEPROM.h>
#include <trigger.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>


// Define our pins
#define thermoDO 4
#define thermoCS 5
#define thermoCLK 6
#define vibrPin 7 // PD7
#define relayPin 8  // PB0
#define dimmerPin 9

//RAM debug
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;


//Init the thermocouple with the appropriate pins defined above with the prefix "thermo"
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
// EasyNextion object init
EasyNex myNex(Serial);

// RobotDYN Dimmer object init
dimmerLamp dimmer(dimmerPin); //initialase port for dimmer for MEGA, Leonardo, UNO, Arduino M0, Arduino Zero




// Global var  - just defined globally to avoid reading the temp sensor in every function i'm using it for
float currentTempReadValue = 0.00;
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
  myNex.begin(115200); //this has been set as an init baud parameter on the Nextion LCD
  Serial.begin(115200); // switching our board to the new serial speed
  dimmer.begin(TOGGLE_MODE, OFF); //dimmer initialisation: name.begin(MODE, STATE)

  dimmer.toggleSettings(0, 100); //Name.toggleSettings(MIN, MAX);
  dimmer.setState(ON); // state: dimmer1.setState(ON/OFF);

  // relay port init and set initial operating mode
  pinMode(vibrPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);

  // Chip side  HIGH/LOW  specification
  // PORTB |= _BV(PB0);
  //  PORTB &= ~_BV(PB0); slow
  // PORTB |= _BV(PB0); //relayPin HIGH
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
  if ( !(tmp1<0)  && tmp1 != NAN) {  // some checks to make sur ewe're getting an acceptable value
    myNex.writeNum("page1.n0.val", tmp1);
  }
  tmp2 = readIntFromEEPROM(EEP_ADDR2);
  if ( !(tmp2<0)  && tmp2 != NAN) {  // some checks to make sur ewe're getting an acceptable value
    myNex.writeNum("page1.n1.val", tmp2);
  }
  tmp3 = readIntFromEEPROM(EEP_ADDR3);
  if ( !(tmp3<0)  && tmp3 != NAN) {  // some checks to make sur ewe're getting an acceptable value
    myNex.writeNum("page2.n0.val", tmp3);
  }
  tmp4 = readIntFromEEPROM(EEP_ADDR4);
  if ( !(tmp4<0)  && tmp4 != NAN) {  // some checks to make sur ewe're getting an acceptable value
    myNex.writeNum("page2.n1.val", tmp4);
  }
  myNex.writeNum("page0.n1.val", getFreeSram());
}


//Main loop where all the bellow logic is continuously run
void loop() {
  myNex.NextionListen();
  // Reading the temperature every 250ms between the loops
  if ((millis() - timer) > 350UL){
    currentTempReadValue = thermocouple.readCelsius();
    if (currentTempReadValue == NAN || currentTempReadValue < 0) currentTempReadValue = thermocouple.readCelsius();  // Making sure we're getting a desired value
    timer += 350UL;
  }
  doCoffee();
}


//  ALL used functions declared bellow
// The *precise* temp control logic
void doCoffee() {
  // Getting the latest LCD side set temp settings 
  if (myNex.currentPageId != 0 && lcdValuesUpdated == false) {
    // Making sure the serial communication finishes sending all the values
    setPoint = myNex.readNumber("page1.n0.val");  // reading the setPoint value from the lcd
    if (!(setPoint<0) && setPoint != NAN && setPoint > 85) {
      delay(0);
    } else {
      setPoint = myNex.readNumber("page1.n0.val");
    }

    offsetTemp = myNex.readNumber("page1.n1.val");  // reading the offset value from the lcd
    if ( !(offsetTemp<0) && offsetTemp != NAN ) {
      delay(0);
    } else {
      offsetTemp = myNex.readNumber("page1.n1.val");
    }

    brewTimeDelayTemp = myNex.readNumber("page2.n0.val");  // reading the brew time delay used to apply heating in waves
    if ( !(brewTimeDelayTemp<0) && brewTimeDelayTemp != NAN ) {
      delay(0);
    } else {
      brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
    }

    brewTimeDelayDivider = myNex.readNumber("page2.n1.val");  // reading the delay divider
    if ( !(brewTimeDelayDivider<0) && brewTimeDelayDivider != NAN ) {
      delay(0);
    } else {
      brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
    }
    lcdValuesUpdated == true; // setting this to TRUE so the condition is skipped on subsequent iterations
  } else if ( myNex.currentPageId == 0 && lcdValuesUpdated == false) {
    // Making sure the serial communication finishes sending all the values
    setPoint = myNex.readNumber("page1.n0.val");  // reading the setPoint value from the lcd
    if (!(setPoint<0) && setPoint != NAN && setPoint > 85) {
      delay(0);
    } else {
      setPoint = myNex.readNumber("page1.n0.val");
    }

    offsetTemp = myNex.readNumber("page1.n1.val");  // reading the offset value from the lcd
    if ( !(offsetTemp<0) && offsetTemp != NAN ) {
      delay(0);
    } else {
      offsetTemp = myNex.readNumber("page1.n1.val");
    }

    brewTimeDelayTemp = myNex.readNumber("page2.n0.val");  // reading the brew time delay used to apply heating in waves
    if ( !(brewTimeDelayTemp<0) && brewTimeDelayTemp != NAN ) {
      delay(0);
    } else {
      brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
    }

    brewTimeDelayDivider = myNex.readNumber("page2.n1.val");  // reading the delay divider
    if ( !(brewTimeDelayDivider<0) && brewTimeDelayDivider != NAN ) {
      delay(0);
    } else {
      brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
    }
    lcdValuesUpdated = true; // setting this to TRUE so the condition is skipped on subsequent iterations
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
    delay(powerOutput);  // delaying the relayPin state for <powerOutput> ammount of time
    PORTB &= ~_BV(PB0);  // relayPIN -> LOW
    delay(powerOutput);  // delaying the relayPin state for <powerOutput> ammount of time
  } else {
    PORTB &= ~_BV(PB0);  // relayPIN -> LOW
  }

  // Updating the LCD every 500ms
  if (myNex.currentPageId == 0) {
    if ((millis() - timer_lcd) > 500UL){
      if (currentTempReadValue < 115.00 ) {
        if (powerOutput > brewTimeDelayTemp) {
          powerOutput = brewTimeDelayTemp;
        } else if (powerOutput < brewTimeDelayTemp / brewTimeDelayDivider) {
          powerOutput = brewTimeDelayTemp / brewTimeDelayDivider;
        } else {
          myNex.writeNum("page0.n0.val", powerOutput);
        }
        myNex.writeNum("page0.n0.val", powerOutput);
      } else if (currentTempReadValue > 115.00 && !(currentTempReadValue < 0) && currentTempReadValue != NAN ) {
        static bool blink = true;
        static unsigned long timer_s1 = millis(), timer_s2 = millis();
        unsigned long currentMillis = millis();
        if (blink == true) {
          if (currentMillis >= timer_s1 + 1000UL) {
            timer_s1 += 1000UL;
            blink = false;
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


// Vibrtion sensor
// bool vibrSense() {
//   bool vibration_detect = false;
//   unsigned long vibration = pulseIn(vibrPin, HIGH);
//   if (vibration > 2000) vibration_detect = true;
//   return vibration_detect;
// }

// Pump dimming during for preinfusion
void preinfusion() {
  //dealing with preinfusion here, might take more time to get it right
 if (vibrPin != 0 && preinfusionState(myNex.readNumber("page2.c0.val")) == true) {
   for (unsigned long i=millis();i > millis()+5000UL;) {
     dimmer.setPower(50);
     doCoffee();
   }
   dimmer.setPower(100);
  //  preinfusionState(false);
 }
}

void pressureProfile() {
  //do smth in the future
}

void descale() {
  //do smth
}

bool preinfusionState(byte c) {
  static bool pageRead = false;
  if (myNex.currentPageId == 0 && pageRead == false) {
    if (c == 0) return false;
    if (c == 1) return true;
  }
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
