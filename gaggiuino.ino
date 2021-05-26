#include <EEPROM.h>
#include <trigger.h>
#include <EasyNextionLibrary.h>
#include <max6675.h>

// Define our pins
int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;
int relayPin = 8;

//Init the thermocouple with the appropriate pins
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
EasyNex myNex(Serial);

// EEPROM  stuff
int EEP_ADDR1 = 1, EEP_ADDR2 = 20, EEP_ADDR3 = 40, EEP_ADDR4 = 60;
unsigned long EEP_VALUE1, EEP_VALUE2, EEP_VALUE3, EEP_VALUE4;

int i = 0;
uint32_t setPoint, offsetTemp, brewTimeDelayTemp, brewTimeDelayDivider, realTempRead, waterTemp;
float CurrentTempReadValue;
float const MAX_TEMP = 120;
const int MAX = 100;


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

// Button minus
void trigger1()
{
  uint32_t setPoint;
  setPoint = myNex.readNumber("page1.n0.val");
  setPoint--;
  myNex.writeNum("page1.n0.val",setPoint);
}
// Button plus
void trigger2()
{
  uint32_t setPoint;
  setPoint = myNex.readNumber("page1.n0.val");
  setPoint++;
  myNex.writeNum("page1.n0.val",setPoint);
}
// Button minus
void trigger3()
{
  uint32_t offsetTemp;
  offsetTemp = myNex.readNumber("page1.n1.val");
  offsetTemp--;
  myNex.writeNum("page1.n1.val",offsetTemp);
}
// Button plus
void trigger4()
{
  uint32_t offsetTemp;
  offsetTemp = myNex.readNumber("page1.n1.val");
  offsetTemp++;
  myNex.writeNum("page1.n1.val",offsetTemp);
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

// The *precise* temp control logic
void doCoffee() {
  // Getting the operational readings
  CurrentTempReadValue = thermocouple.readCelsius();
  setPoint = myNex.readNumber("page1.n0.val");
  offsetTemp = myNex.readNumber("page1.n1.val");
  brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  brewTimeDelayDivider = myNex.readNumber("page2.n1.val");

  // Calculating the boiler heating power
  int powerOutput = map(CurrentTempReadValue, setPoint-10, setPoint, brewTimeDelayTemp, brewTimeDelayTemp/brewTimeDelayDivider);
  if (powerOutput > brewTimeDelayTemp){
    powerOutput = brewTimeDelayTemp;
  }
  else if (powerOutput < brewTimeDelayTemp/brewTimeDelayDivider) {
    powerOutput = brewTimeDelayTemp/brewTimeDelayDivider;
  }

  // Applying the powerOutput variable as part of the relay logic
  if (thermocouple.readCelsius() < float(setPoint-10)) {
    digitalWrite(relayPin, HIGH);
  }
  else if (thermocouple.readCelsius() >= float(setPoint-10) && thermocouple.readCelsius() < float(setPoint-3)) {
    digitalWrite(relayPin, HIGH);
    delay(powerOutput);
    digitalWrite(relayPin, LOW);
  }
  else if (thermocouple.readCelsius() >= float(setPoint-3) && thermocouple.readCelsius() <= float(setPoint-1)) {
    digitalWrite(relayPin, HIGH);
    delay(powerOutput);
    digitalWrite(relayPin, LOW);
    delay(powerOutput);
  }
  else if (thermocouple.readCelsius() >= float(setPoint-1) && thermocouple.readCelsius() < float(setPoint-0.2)) {
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
  CurrentTempReadValue = thermocouple.readCelsius();
  setPoint = myNex.readNumber("page1.n0.val");
  offsetTemp = myNex.readNumber("page1.n1.val");
  brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
  
  // Calculating the boiler heating power to apply
  int powerOutput = map(CurrentTempReadValue, setPoint-10, setPoint, brewTimeDelayTemp, brewTimeDelayTemp/brewTimeDelayDivider);
  if (powerOutput > brewTimeDelayTemp){
    powerOutput = brewTimeDelayTemp;
  }
  else if (powerOutput < brewTimeDelayTemp/brewTimeDelayDivider) {
    powerOutput = brewTimeDelayTemp/brewTimeDelayDivider;
  }

  //Printing the current values to the display
  waterTempPrint = CurrentTempReadValue - float(offsetTemp);
  hPwrPrint = powerOutput;

  myNex.writeStr("page0.t0.txt", waterTempPrint);
  myNex.writeStr("page0.t1.txt", hPwrPrint);
}
void setup() {
  
  myNex.begin(9600);  
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
  // port init - starts with the relay decoupled just in case
  digitalWrite(relayPin, LOW);

  // Small delay so the LCD has a chance to fully initialize before passing the EEPROM values
  delay(900);

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
}

void loop() {
  //Main loop where all the above logic is continuously run
  myNex.NextionListen();
  doCoffee();
  updateLCD();
  delay(300);
}
