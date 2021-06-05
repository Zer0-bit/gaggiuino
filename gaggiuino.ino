#include <RBDdimmer.h>
#include <EEPROM.h>
#include <trigger.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>


// Define our pins
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
int vibrPin = 7; // PD7
int relayPin = 8; // PB0


//Init the thermocouple with the appropriate pins defined above with the prefix "thermo"
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
EasyNex myNex(Serial);

// EEPROM  stuff
const int EEP_ADDR1 = 1, EEP_ADDR2 = 20, EEP_ADDR3 = 40, EEP_ADDR4 = 60;

// Some globals vars used for logic blocks mostly
float currentTempReadValue = 0;
unsigned long timer = 0;
const unsigned long interval = 250;
const float STEAM_START = 115.00;





void setup() {
  myNex.begin(9600);
  //Registering a higher baud rate for quicker serial communication
  Serial.print("baud=115200");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.end();
  Serial.begin(115200);

  // relay port init and set initial operating mode
  pinMode(vibrPin, INPUT);
  pinMode(relayPin, OUTPUT);

  // Chip side  HIGH/LOW  specification
  // PORTB |= _BV(PB0);
  //  PORTB &= ~_BV(PB0); slow
  // PORTB |= _BV(PB0); //relayPin HIGH
  PORTB &= ~_BV(PB0);// relayPin LOW

  // Will wait hereuntil full serial is established, this is done so the LCD fully initializes before passing the EEPROM values
  delay(1000);
  // Applying the EEPROM saved values
  uint32_t tmp1 = 0;
  uint32_t tmp2 = 0;
  uint32_t tmp3 = 0;
  uint32_t tmp4 = 0;

  // Making sure we're getting the right values and sending them to the display
  tmp1 = readIntFromEEPROM(EEP_ADDR1);
  if (tmp1 != 777777 && tmp1>0) {  // 777777 is the return value if the incoming value is malformed
    myNex.writeNum("page1.n0.val", tmp1);
  } 
  // else {
  //   tmp1 = readIntFromEEPROM(EEP_ADDR1);
  //   myNex.writeNum("page1.n0.val", tmp1);
  // }
  tmp2 = readIntFromEEPROM(EEP_ADDR2);
  if (tmp2 != 777777 && tmp2>0) {  // 777777 is the return value if the incoming value is malformed
    myNex.writeNum("page1.n1.val", tmp2);
  } 
  // else {
  //   tmp2 = readIntFromEEPROM(EEP_ADDR2);
  //   myNex.writeNum("page1.n1.val", tmp2);
  // }
  tmp3 = readIntFromEEPROM(EEP_ADDR3);
  if (tmp3 != 777777 && tmp3>0) {  // 777777 is the return value if the incoming value is malformed
    myNex.writeNum("page2.n0.val", tmp3);
  } 
  // else {
  //   tmp3 = readIntFromEEPROM(EEP_ADDR3);
  //   myNex.writeNum("page2.n0.val", tmp3);
  // }
  tmp4 = readIntFromEEPROM(EEP_ADDR4);
  if (tmp4 != 777777 && tmp4>0) {  // 777777 is the return value if the incoming value is malformed
    myNex.writeNum("page2.n1.val", tmp4);
  } 
  // else {
  //   tmp4 = readIntFromEEPROM(EEP_ADDR4);
  //   myNex.writeNum("page2.n1.val", tmp4);
  // }
}


//Main loop where all the bellow logic is continuously run
void loop() {
  // Reading the temperature just once ever 250ms between the loops
  if (millis() - timer >= interval) {
    timer += interval;
    currentTempReadValue = thermocouple.readCelsius();
    if (currentTempReadValue == NAN || currentTempReadValue < 0) currentTempReadValue = thermocouple.readCelsius();  // Making sure we're getting a expected value
  }
  myNex.NextionListen();
  doCoffee();
}


//  ALL used functions declared bellow
// The *precise* temp control logic
void doCoffee() {
  int setPoint = 0;
  int offsetTemp = 0;
  int brewTimeDelayTemp = 0;
  int brewTimeDelayDivider = 0;
  char waterTempPrint[8];  

  // Making sure the serial communication finishes sending all the values
  setPoint = myNex.readNumber("page1.n0.val");  // reading the setPoint value from the lcd
  if (setPoint != 777777 && setPoint>0) {
    delay(0);
  } else if (setPoint == 777777 || setPoint<0) {
    setPoint = myNex.readNumber("page1.n0.val");
  }

  offsetTemp = myNex.readNumber("page1.n1.val");  // reading the offset value from the lcd
  if (offsetTemp != 777777 && offsetTemp>0) {
    delay(0);
  } else if (offsetTemp == 777777 || offsetTemp<0) {
    offsetTemp = myNex.readNumber("page1.n1.val");
  }

  brewTimeDelayTemp = myNex.readNumber("page2.n0.val");  // reading the brew time delay used to apply heating in waves
  if (brewTimeDelayTemp != 777777 && brewTimeDelayTemp > 0) {
    delay(0);
  } else if (brewTimeDelayTemp == 777777 || brewTimeDelayTemp < 0) {
    brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  }

  brewTimeDelayDivider = myNex.readNumber("page2.n1.val");  // reading the delay divider
  if (brewTimeDelayDivider != 777777 && brewTimeDelayDivider > 0) {
    delay(0);
  } else if (brewTimeDelayDivider == 777777 || brewTimeDelayDivider < 0) {
    brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
  }
  // Calculating the boiler heating power
  int powerOutput = map(currentTempReadValue, setPoint - 10, setPoint, brewTimeDelayTemp, brewTimeDelayTemp / brewTimeDelayDivider);
  if (powerOutput > brewTimeDelayTemp) {
    powerOutput = brewTimeDelayTemp;
  } else if (powerOutput < brewTimeDelayTemp / brewTimeDelayDivider) {
    powerOutput = brewTimeDelayTemp / brewTimeDelayDivider;
  }

  // Applying the powerOutput variable as part of the relay logic
  if (currentTempReadValue < float(setPoint - 10) && !(currentTempReadValue < 0) && currentTempReadValue != NAN) {
    PORTB |= _BV(PB0); // relayPIN -> HIGH
  } else if (currentTempReadValue >= float(setPoint - 10) && currentTempReadValue < float(setPoint - 3)) {
    PORTB |= _BV(PB0); // relayPIN -> HIGH
    delay(powerOutput);
     PORTB &= ~_BV(PB0); // relayPIN -> LOW
  } else if (currentTempReadValue >= float(setPoint - 3) && currentTempReadValue <= float(setPoint - 1)) {
    PORTB |= _BV(PB0); // relayPIN -> HIGH
    delay(powerOutput);
     PORTB &= ~_BV(PB0); // relayPIN -> LOW
    delay(powerOutput);
  } else if (currentTempReadValue >= float(setPoint - 1) && currentTempReadValue < float(setPoint - 0.2)) {
    PORTB |= _BV(PB0); // relayPIN -> HIGH
    delay(powerOutput);
     PORTB &= ~_BV(PB0); // relayPIN -> LOW
    delay(powerOutput);
  } else {
     PORTB &= ~_BV(PB0); // relayPIN -> LOW
  }

  // Updating the LCD
  if (currentTempReadValue < STEAM_START) {
    if (powerOutput > brewTimeDelayTemp) {
      powerOutput = brewTimeDelayTemp;
    } else if (powerOutput < brewTimeDelayTemp / brewTimeDelayDivider) {
      powerOutput = brewTimeDelayTemp / brewTimeDelayDivider;
    } else {
      myNex.writeNum("page0.n0.val", powerOutput);
    }
    myNex.writeNum("page0.n0.val", powerOutput);
  } else if (currentTempReadValue > STEAM_START && !(currentTempReadValue < 0) && currentTempReadValue != NAN) {
    static bool blink = true;
    static unsigned long timer_s1 = 0, timer_s2 = 0;
    unsigned long currentMillis = millis();
    if (blink == true) {
      if (currentMillis - timer_s1 >= 1000UL) {
        timer_s1 = currentMillis;
        blink = false;
      }
      myNex.writeStr("vis t1,1");
      myNex.writeNum("page0.n0.val", NAN);
      dtostrf(currentTempReadValue - offsetTemp, 6, 2, waterTempPrint);
      myNex.writeStr("page0.t0.txt", waterTempPrint);
    } else {
        timer_s2 = currentMillis;
        myNex.writeStr("vis t1,0");
        myNex.writeNum("page0.n0.val", NAN);
        dtostrf(currentTempReadValue - offsetTemp, 6, 2, waterTempPrint);
        myNex.writeStr("page0.t0.txt", waterTempPrint);
        if (currentMillis - timer_s1  >= 1000UL) blink = true;
    }
  } else {
    myNex.writeNum("page0.n0.val", NAN);
  }
  dtostrf(currentTempReadValue - offsetTemp, 6, 2, waterTempPrint);
  myNex.writeStr("page0.t0.txt", waterTempPrint); // Printing the current water temp values to the display
}

// EEPROM WRITE
void writeIntIntoEEPROM(int address, int number) {
  EEPROM.update(address, number >> 8);
  EEPROM.update(address + 1, number & 0xFF);
}
//EEPROM READ
int readIntFromEEPROM(int address) {
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

// Save the desired temp values to EEPROM
void trigger1() {
  int savedBoilerTemp = 0;
  int savedOffsetTemp = 0;
  int savedBrewTimeDelay = 0;
  int savedbrewTimeDivider = 0;

  // Reading our values from the LCD and checking whether we got proper serial communication
  savedBoilerTemp = myNex.readNumber("page1.n0.val");
  if (savedBoilerTemp != 777777 && savedBoilerTemp > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  } 
  // else if (savedBoilerTemp == 777777 || savedBoilerTemp < 0) {
  //   savedBoilerTemp = myNex.readNumber("page1.n0.val");
  //   writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
  //   myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
  //   myNex.writeStr("page popupMSG");
  // }


  savedOffsetTemp = myNex.readNumber("page1.n1.val");
  if (savedOffsetTemp != 777777 && savedOffsetTemp > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  } 
  // else if (savedOffsetTemp == 777777 || savedOffsetTemp < 0) {
  //   savedOffsetTemp = myNex.readNumber("page1.n1.val");
  //   writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
  //   myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
  //   myNex.writeStr("page popupMSG");
  // }


  savedBrewTimeDelay = myNex.readNumber("page2.n0.val");
  if (savedBrewTimeDelay != 777777 && savedBrewTimeDelay > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  }
  //  else if (savedBrewTimeDelay == 777777 || savedBrewTimeDelay < 0) {
  //   savedBrewTimeDelay = myNex.readNumber("page2.n0.val");
  //   writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
  //   myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
  //   myNex.writeStr("page popupMSG");
  // }


  savedbrewTimeDivider = myNex.readNumber("page2.n1.val");
  if (savedbrewTimeDivider != 777777 && savedbrewTimeDivider > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR4, savedbrewTimeDivider);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  }
  
  //  else if (savedbrewTimeDivider == 777777 || savedbrewTimeDivider < 0) {
  //   savedbrewTimeDivider = myNex.readNumber("page2.n1.val");
  //   writeIntIntoEEPROM(EEP_ADDR4, savedbrewTimeDivider);
  //   myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
  //   myNex.writeStr("popupMSG.t0.pco=BLACK");
  //   myNex.writeStr("page popupMSG");
  // }
}

// Vibrtion sensor
bool vibrSense() {
  bool vibration_detect = false;
  unsigned long vibration = pulseIn(vibrPin, HIGH);
  if (vibration > 2000) vibration_detect = true;
  return vibration_detect;
}
