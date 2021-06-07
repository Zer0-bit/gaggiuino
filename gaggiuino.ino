// #include <RBDdimmer.h>
#include <EEPROM.h>
#include <trigger.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>


// Define our pins
#define thermoDO 4
#define thermoCS 5
#define thermoCLK 6
//#define vibrPin 7 // PD7
#define relayPin 8// PB0

//RAM debug
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__


//Init the thermocouple with the appropriate pins defined above with the prefix "thermo"
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
EasyNex myNex(Serial);



// Some globals vars used for logic blocks mostly
float currentTempReadValue = 0.00;
unsigned long timer = 0;

byte setPoint = 0;
byte offsetTemp = 0;
uint16_t brewTimeDelayTemp = 0;
byte brewTimeDelayDivider = 0;
char waterTempPrint[6];
// EEPROM  stuff
int EEP_ADDR1 = 1, EEP_ADDR2 = 20, EEP_ADDR3 = 40, EEP_ADDR4 = 60;


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
  // pinMode(vibrPin, INPUT);
  pinMode(relayPin, OUTPUT);

  // Chip side  HIGH/LOW  specification
  // PORTB |= _BV(PB0);
  //  PORTB &= ~_BV(PB0); slow
  // PORTB |= _BV(PB0); //relayPin HIGH
  PORTB &= ~_BV(PB0);// relayPin LOW

  // Will wait hereuntil full serial is established, this is done so the LCD fully initializes before passing the EEPROM values
  delay(500);
  // Applying the EEPROM saved values
  uint8_t tmp1 = 0;
  uint8_t tmp2 = 0;
  uint16_t tmp3 = 0;
  uint8_t tmp4 = 0;

  // Making sure we're getting the right values and sending them to the display
  tmp1 = readIntFromEEPROM(EEP_ADDR1);
  if (tmp1 != 777777 && tmp1>0) {  // 777777 is the return value if the incoming value is malformed
    myNex.writeNum("page1.n0.val", tmp1);
  } 
  tmp2 = readIntFromEEPROM(EEP_ADDR2);
  if (tmp2 != 777777 && tmp2>0) {  // 777777 is the return value if the incoming value is malformed
    myNex.writeNum("page1.n1.val", tmp2);
  } 
  tmp3 = readIntFromEEPROM(EEP_ADDR3);
  if (tmp3 != 777777 && tmp3>0) {  // 777777 is the return value if the incoming value is malformed
    myNex.writeNum("page2.n0.val", tmp3);
  } 
  tmp4 = readIntFromEEPROM(EEP_ADDR4);
  if (tmp4 != 777777 && tmp4>0) {  // 777777 is the return value if the incoming value is malformed
    myNex.writeNum("page2.n1.val", tmp4);
  } 
}


//Main loop where all the bellow logic is continuously run
void loop() {
  // Reading the temperature every 250ms between the loops
  if (millis() >= timer + 250UL) {
    timer += 250UL;
    currentTempReadValue = thermocouple.readCelsius();
    if (currentTempReadValue == NAN || currentTempReadValue < 0) currentTempReadValue = thermocouple.readCelsius();  // Making sure we're getting a desired value
  }
  myNex.NextionListen();
  doCoffee();
}


//  ALL used functions declared bellow
// The *precise* temp control logic
void doCoffee() {
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
  uint16_t powerOutput = map(currentTempReadValue, setPoint - 10, setPoint, brewTimeDelayTemp, brewTimeDelayTemp / brewTimeDelayDivider);
  if (powerOutput > brewTimeDelayTemp) {
    powerOutput = brewTimeDelayTemp;
  } else if (powerOutput < brewTimeDelayTemp / brewTimeDelayDivider) {
    powerOutput = brewTimeDelayTemp / brewTimeDelayDivider;
  }

  // Applying the powerOutput variable as part of the relay logic
  if (currentTempReadValue < (float)setPoint - 10.00 && !(currentTempReadValue < 0.00) && currentTempReadValue != NAN) {
    PORTB |= _BV(PB0); // relayPIN -> HIGH
  } else if (currentTempReadValue >= (float)setPoint - 10.00 && currentTempReadValue < (float)setPoint - 3.00) {
    PORTB |= _BV(PB0); // relayPIN -> HIGH
    delay(powerOutput);
     PORTB &= ~_BV(PB0); // relayPIN -> LOW
  } else if (currentTempReadValue >= (float)setPoint - 3.00 && currentTempReadValue <= float(setPoint - 1.00)) {
    PORTB |= _BV(PB0); // relayPIN -> HIGH
    delay(powerOutput);
     PORTB &= ~_BV(PB0); // relayPIN -> LOW
    delay(powerOutput);
  } else if (currentTempReadValue >= (float)setPoint - 1.00 && currentTempReadValue < (float)setPoint - 0.50) {
    PORTB |= _BV(PB0); // relayPIN -> HIGH
    delay(powerOutput);
     PORTB &= ~_BV(PB0); // relayPIN -> LOW
    delay(powerOutput);
  } else {
     PORTB &= ~_BV(PB0); // relayPIN -> LOW
  }

  // Updating the LCD
  if (currentTempReadValue < 115.00) {
    if (powerOutput > brewTimeDelayTemp) {
      powerOutput = brewTimeDelayTemp;
    } else if (powerOutput < brewTimeDelayTemp / brewTimeDelayDivider) {
      powerOutput = brewTimeDelayTemp / brewTimeDelayDivider;
    } else {
      myNex.writeNum("page0.n0.val", powerOutput);
    }
    myNex.writeNum("page0.n0.val", powerOutput);
  } else if (currentTempReadValue > 115.00 && !(currentTempReadValue < 0) && currentTempReadValue != NAN) {
    static bool blink = true;
    static unsigned long timer_s1 = 0, timer_s2 = 0;
    unsigned long currentMillis = millis();
    if (blink == true) {
      if (currentMillis >= timer_s1 + 1000UL) {
        timer_s1 += 1000UL;
        blink = false;
      }
      myNex.writeStr("vis t1,1");
      myNex.writeNum("page0.n0.val", NAN);
      dtostrf(currentTempReadValue - (float)offsetTemp, 6, 2, waterTempPrint);
      myNex.writeStr("page0.t0.txt", waterTempPrint);
    } else {
        timer_s2 = currentMillis;
        myNex.writeStr("vis t1,0");
        myNex.writeNum("page0.n0.val", NAN);
        dtostrf(currentTempReadValue - (float)offsetTemp, 6, 2, waterTempPrint);
        myNex.writeStr("page0.t0.txt", waterTempPrint);
        if (currentMillis >= timer_s2 + 1000UL) blink = true;
    }
  } else {
    myNex.writeNum("page0.n0.val", NAN);
  }
  dtostrf(currentTempReadValue - (float)offsetTemp, 6, 2, waterTempPrint);
  myNex.writeStr("page0.t0.txt", waterTempPrint); // Printing the current water temp values to the display
  myNex.writeNum("page0.n1.val", freeMemory());
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

  if(EEP_ADDR1 >=19) EEP_ADDR1 = 1;
  if(EEP_ADDR2 >=39) EEP_ADDR1 = 20;
  if(EEP_ADDR3 >=59) EEP_ADDR1 = 40;
  if(EEP_ADDR4 >=79) EEP_ADDR1 = 60;

  // Reading our values from the LCD and checking whether we got proper serial communication
  if (savedBoilerTemp != 777777 && savedBoilerTemp > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
    delay(50);
  } 
  if (savedOffsetTemp != 777777 && savedOffsetTemp > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
    delay(50);
  } 
  if (savedBrewTimeDelay != 777777 && savedBrewTimeDelay > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
    delay(50);
  }
  if (savedbrewTimeDivider != 777777 && savedbrewTimeDivider > 0) {  // 777777 is the return value if the incoming value is malformed
    writeIntIntoEEPROM(EEP_ADDR4, savedbrewTimeDivider);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
    delay(50);
  }
}

// Vibrtion sensor
// bool vibrSense() {
//   bool vibration_detect = false;
//   unsigned long vibration = pulseIn(vibrPin, HIGH);
//   if (vibration > 2000) vibration_detect = true;
//   return vibration_detect;
// }

int freeMemory() {
  char top;
  #ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
  #elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
  #else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
  #endif  // __arm__
}
