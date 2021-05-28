#include <EEPROM.h>
#include <trigger.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>
#include <RBDdimmer.h>

// Define our pins
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
bool vibroPin = 7;
int relayPin = 8;
bool preinfusionEnabled;

// Dimmer stuff
#define outputPin  9 
#define zerocross  5

//Init the thermocouple with the appropriate pins
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
EasyNex myNex(Serial);
dimmerLamp dimmer(outputPin);

// EEPROM  stuff
int EEP_ADDR1 = 1, EEP_ADDR2 = 20, EEP_ADDR3 = 40, EEP_ADDR4 = 60;
unsigned long EEP_VALUE1, EEP_VALUE2, EEP_VALUE3, EEP_VALUE4;

int i = 0, preinfusionRunCheck = 0;
uint32_t setPoint, offsetTemp, brewTimeDelayTemp, brewTimeDelayDivider, realTempRead, waterTemp;
float const MAX_TEMP = 120;
const int MAX = 100;

// Main loop
void setup() {
  // Serial communication init
  myNex.begin(9600);

  // Port init - starts with the relay decoupled
  digitalWrite(relayPin, LOW);

  // Dimmer lib init
  dimmer.begin(NORMAL_MODE, ON);
  // nexInit();
  delay(100);
  //Registering a higher baud for hopefully more responsive touch controls
  Serial.print("baud=115200");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.end();
  Serial.begin(115200);

  // port setup
  pinMode(relayPin, OUTPUT);
  pinMode(vibroPin, INPUT); 

  // Small delay so the LCD has a chance to fully initialize before passing the EEPROM values
  delay(600);

  preinfusionEnabled = myNex.readNumber("c0");
  // Applying the EEPROM saved values  
  uint32_t tmp1 = readIntFromEEPROM(EEP_ADDR1);
  uint32_t tmp2 = readIntFromEEPROM(EEP_ADDR2);
  uint32_t tmp3 = readIntFromEEPROM(EEP_ADDR3);
  uint32_t tmp4 = readIntFromEEPROM(EEP_ADDR4);
  if ((tmp1 != 0) && (tmp2 != 0) && (tmp3 != 0) && (tmp4 != 0)) {
    myNex.writeNum("page1.n0.val",tmp1);
    myNex.writeNum("page1.n1.val",tmp2);
    myNex.writeNum("page2.n0.val",tmp3);
    myNex.writeNum("page2.n1.val",tmp4);
  }
  delay(50);
}

void loop() {
  //Main loop where all the above logic is continuously run
  myNex.NextionListen();
  do {
    doCoffee();    
  }
  while (preinfusionEnabled == false);
  do {
    doCoffeeWithPreinfusion();
  }
  while (preinfusionEnabled == true);
  updateLCD();
  preinfusionStop();
  delay(250);
}

// EEPROM WRITE
void writeIntIntoEEPROM(int address, int number)
{ 
  EEPROM.update(address, number >> 8);
  EEPROM.update(address + 1, number & 0xFF);
}

//EEPROM READ
int readIntFromEEPROM(int address)
{
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

// Save the desired temp values to EEPROM
void trigger5()
{
  int savedBoilerTemp = myNex.readNumber("page1.n0.val");
  int savedOffsetTemp = myNex.readNumber("page1.n1.val");
  int savedBrewTimeDelay = myNex.readNumber("page2.n0.val");
  int savedbrewTimeDivider = myNex.readNumber("page2.n1.val");

  writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
  writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
  writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
  writeIntIntoEEPROM(EEP_ADDR4, savedbrewTimeDivider);
  myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
  myNex.writeStr("page popupMSG");
}

// Preinfusion logic
void preinfusion() {
  int preinfuseTimer = myNex.readNumber("page2.n1.val");
  while (preinfusionRunCheck < 1) {
    uint32_t period = preinfuseTimer * 1000;  // calc seconds
    // Dim the pump for *period ammount of seconds
    for( uint32_t tStart = millis();  (millis()-tStart) < period;  ){
      dimmer.setPower(20);
    }
    
    uint32_t pause = 3 * 1000;  // calc seconds
    // Dim the pump for *pause ammount of seconds
    for( uint32_t tStart = millis();  (millis()-tStart) < pause;  ){
      dimmer.setPower(0);
    }
    dimmer.setPower(100);
    preinfusionRunCheck++;
  }
}

// Detect when brewing stops
void preinfusionStop() {
  if (vibroPin != true) preinfusionRunCheck = 0;
}

// EXPERIMENTAL NOT IN USE NOW 
void doCoffeeWithPreinfusion() {
  // Getting the operational readings
  float currentTempReadValue = thermocouple.readCelsius();
  setPoint = myNex.readNumber("page1.n0.val");
  offsetTemp = myNex.readNumber("page1.n1.val");
  brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  brewTimeDelayDivider = myNex.readNumber("page2.n1.val");

  // Calculating the boiler heating power
  int powerOutput = map(currentTempReadValue, setPoint-10, setPoint, brewTimeDelayTemp, brewTimeDelayTemp/brewTimeDelayDivider);
  while (vibroPin == true ) {
    preinfusion();
    if (powerOutput > brewTimeDelayTemp){
      powerOutput = brewTimeDelayTemp;
    }
    else if (powerOutput < brewTimeDelayTemp/brewTimeDelayDivider) {
      powerOutput = brewTimeDelayTemp/brewTimeDelayDivider;
    }

    // Applying the powerOutput variable as part of the relay logic
    if (currentTempReadValue < float(setPoint-10)) {
      digitalWrite(relayPin, HIGH);
    }
    else if (currentTempReadValue >= float(setPoint-10) && currentTempReadValue < float(setPoint-3)) {
      digitalWrite(relayPin, HIGH);
      delay(powerOutput);
      digitalWrite(relayPin, LOW);
    }
    else if (currentTempReadValue >= float(setPoint-3) && currentTempReadValue <= float(setPoint-1)) {
      digitalWrite(relayPin, HIGH);
      delay(powerOutput);
      digitalWrite(relayPin, LOW);
      delay(powerOutput);
    }
    else if (currentTempReadValue >= float(setPoint-1) && currentTempReadValue < float(setPoint-0.2)) {
      digitalWrite(relayPin, HIGH);
      delay(powerOutput);
      digitalWrite(relayPin, LOW);
      delay(powerOutput);
    }
    else {
      digitalWrite(relayPin, LOW);
    }
  }
}

// The *precise* temp control logic
void doCoffee() {
  // Getting the operational readings
  float currentTempReadValue = thermocouple.readCelsius();
  setPoint = myNex.readNumber("page1.n0.val");
  offsetTemp = myNex.readNumber("page1.n1.val");
  brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  brewTimeDelayDivider = myNex.readNumber("page2.n1.val");

  // Calculating the boiler heating power
  int powerOutput = map(currentTempReadValue, setPoint-10, setPoint, brewTimeDelayTemp, brewTimeDelayTemp/brewTimeDelayDivider);
  if (powerOutput > brewTimeDelayTemp){
    powerOutput = brewTimeDelayTemp;
  }
  else if (powerOutput < brewTimeDelayTemp/brewTimeDelayDivider) {
    powerOutput = brewTimeDelayTemp/brewTimeDelayDivider;
  }

  // Applying the powerOutput variable as part of the relay logic
  if (currentTempReadValue < float(setPoint-10)) {
    digitalWrite(relayPin, HIGH);
  }
  else if (currentTempReadValue >= float(setPoint-10) && currentTempReadValue < float(setPoint-3)) {
    digitalWrite(relayPin, HIGH);
    delay(powerOutput);
    digitalWrite(relayPin, LOW);
  }
  else if (currentTempReadValue >= float(setPoint-3) && currentTempReadValue <= float(setPoint-1)) {
    digitalWrite(relayPin, HIGH);
    delay(powerOutput);
    digitalWrite(relayPin, LOW);
    delay(powerOutput);
  }
  else if (currentTempReadValue >= float(setPoint-1) && currentTempReadValue < float(setPoint-0.2)) {
    digitalWrite(relayPin, HIGH);
    delay(powerOutput);
    digitalWrite(relayPin, LOW);
    delay(powerOutput);
  }
  else {
    digitalWrite(relayPin, LOW);
  }
}

void updateLCD() {
  // Declaring the local vars to keep the temperature and boiler_power values
  String waterTempPrint, hPwrPrint;

  // Getting the latest readings from the needed endpoints
  float currentTempReadValue = thermocouple.readCelsius();
  setPoint = myNex.readNumber("page1.n0.val");
  offsetTemp = myNex.readNumber("page1.n1.val");
  brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
  
  if (currentTempReadValue < float(setPoint+5)) {
    // Calculating the boiler heating power to apply
    int powerOutput = map(currentTempReadValue, setPoint-10, setPoint, brewTimeDelayTemp, brewTimeDelayTemp/brewTimeDelayDivider);
    if (powerOutput > brewTimeDelayTemp) {
      hPwrPrint = brewTimeDelayTemp;
      myNex.writeStr("page0.t1.txt", hPwrPrint);
    }
    else if (powerOutput < brewTimeDelayTemp/brewTimeDelayDivider) {
      hPwrPrint = brewTimeDelayTemp/brewTimeDelayDivider;
      myNex.writeStr("page0.t1.txt", hPwrPrint);
    }
    else {
      hPwrPrint = powerOutput;
      myNex.writeStr("page0.t1.txt", hPwrPrint);
    }
  }
  else {
    waterTempPrint = currentTempReadValue - float(offsetTemp);
    myNex.writeStr("page0.t1.txt", "STEAM");
  }
  //Printing the current values to the display
  waterTempPrint = currentTempReadValue - float(offsetTemp);
  myNex.writeStr("page0.t0.txt", waterTempPrint);
}
