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
const int EEP_ADDR1 = 1, EEP_ADDR2 = 20, EEP_ADDR3 = 40, EEP_ADDR4 = 60;
// unsigned long EEP_VALUE1, EEP_VALUE2, EEP_VALUE3, EEP_VALUE4;

long asyncDelay = 0;
float const MAX_TEMP = 165;
const int STEAM_START = 100;



void setup() {
  myNex.begin(9600);  
  //Registering a higher baud for hopefully more responsive touch controls
  Serial.print("baud=115200");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.end();
  Serial.begin(115200);

  // relay port init and set initial operating mode
  pinMode(relayPin, OUTPUT);  
  digitalWrite(relayPin, LOW);

  // Small delay so the LCD fully initializes before passing the EEPROM values
  delay(800);

  // Applying the EEPROM saved values  
  uint32_t tmp1 = 0;
  uint32_t tmp2 = readIntFromEEPROM(EEP_ADDR2);
  uint32_t tmp3 = readIntFromEEPROM(EEP_ADDR3);
  uint32_t tmp4 = readIntFromEEPROM(EEP_ADDR4);

  // Making sure we're getting the right values and sending them to the display   
  tmp1 = readIntFromEEPROM(EEP_ADDR1);
  if(tmp1 != 777777){       // 777777is the return value if the code fails to read the new value
    myNex.writeNum("page1.n0.val",tmp1);
  } 
  else if(tmp1 == 777777){
    tmp1 = readIntFromEEPROM(EEP_ADDR1);
    delay(5); //Small delay to hopefully get full serial communication
    myNex.writeNum("page1.n0.val",tmp1);
    delay(5); //Small delay to hopefully get full serial communication
  }
  tmp2 = readIntFromEEPROM(EEP_ADDR2);
  if(tmp2 != 777777){       // 777777is the return value if the code fails to read the new value
    myNex.writeNum("page1.n1.val",tmp2);
  } 
  else if(tmp2 == 777777){
    tmp2 = readIntFromEEPROM(EEP_ADDR2);
    delay(5); //Small delay to hopefully get full serial communication
    myNex.writeNum("page1.n1.val",tmp2);
    delay(5); //Small delay to hopefully get full serial communication
  }
  tmp3 = readIntFromEEPROM(EEP_ADDR3);
  if(tmp3 != 777777){       // 777777is the return value if the code fails to read the new value
    myNex.writeNum("page2.n0.val",tmp3);
  } 
  else if(tmp3 == 777777){
    tmp3 = readIntFromEEPROM(EEP_ADDR3);
    delay(5); //Small delay to hopefully get full serial communication
    myNex.writeNum("page2.n0.val",tmp3);
    delay(5); //Small delay to hopefully get full serial communication
  }
  tmp4 = readIntFromEEPROM(EEP_ADDR4);
  if(tmp4 != 777777){       // 777777is the return value if the code fails to read the new value
    myNex.writeNum("page2.n1.val",tmp4);
  } 
  else if(tmp4 == 777777){
    tmp4 = readIntFromEEPROM(EEP_ADDR4);
    delay(5); //Small delay to hopefully get full serial communication
    myNex.writeNum("page2.n1.val",tmp4);
    delay(5); //Small delay to hopefully get full serial communication
  }
}

void loop() {
  //Main loop where all the above logic is continuously run
  myNex.NextionListen();
  doCoffee();
  updateLCD();
  delay(250);
}


//  ALL used functions declared bellow

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
void trigger1()
{
  int savedBoilerTemp = 0;
  int savedOffsetTemp = 0;
  int savedBrewTimeDelay = 0;
  int savedbrewTimeDivider = 0;

  savedBoilerTemp = myNex.readNumber("page1.n0.val");
  if(savedBoilerTemp != 777777){       // 777777is the return value if the code fails to read the new value
    writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  } 
  else if(savedBoilerTemp == 777777){
    savedBoilerTemp = myNex.readNumber("page1.n0.val");
    delay(5); //Small delay to hopefully get full serial communication
    writeIntIntoEEPROM(EEP_ADDR1, savedBoilerTemp);
    delay(5); //Small delay to hopefully get full serial communication
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  }


  savedOffsetTemp = myNex.readNumber("page1.n1.val");
  if(savedOffsetTemp != 777777){       // 777777is the return value if the code fails to read the new value
    writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  } 
  else if(savedOffsetTemp == 777777){
    savedOffsetTemp = myNex.readNumber("page1.n1.val");
    delay(5); //Small delay to hopefully get full serial communication
    writeIntIntoEEPROM(EEP_ADDR2, savedOffsetTemp);
    delay(5); //Small delay to hopefully get full serial communication
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  }


  savedBrewTimeDelay = myNex.readNumber("page2.n0.val");
  if(savedBrewTimeDelay != 777777){       // 777777is the return value if the code fails to read the new value
    writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  } 
  else if(savedBrewTimeDelay == 777777){
    savedBrewTimeDelay = myNex.readNumber("page2.n0.val");
    delay(5); //Small delay to hopefully get full serial communication
    writeIntIntoEEPROM(EEP_ADDR3, savedBrewTimeDelay);
    delay(5); //Small delay to hopefully get full serial communication
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  }


  savedbrewTimeDivider = myNex.readNumber("page2.n1.val");
  if(savedbrewTimeDivider != 777777){       // 777777is the return value if the code fails to read the new value
    writeIntIntoEEPROM(EEP_ADDR4, savedbrewTimeDivider);
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  } 
  else if(savedbrewTimeDivider == 777777){
    savedbrewTimeDivider = myNex.readNumber("page2.n1.val");
    delay(5); //Small delay to hopefully get full serial communication
    writeIntIntoEEPROM(EEP_ADDR4, savedbrewTimeDivider);
    delay(5); //Small delay to hopefully get full serial communication
    myNex.writeStr("popupMSG.t0.txt", "SUCCESS!");
    myNex.writeStr("page popupMSG");
  }
}

// The *precise* temp control logic
void doCoffee() {
  float currentTempReadValue = 0;
  int setPoint = 0;
  int offsetTemp = 0;
  int brewTimeDelayTemp = 0;
  int brewTimeDelayDivider = 0;

  // Making sure the serial communication finishes sending all the values
  currentTempReadValue = thermocouple.readCelsius();
  if (currentTempReadValue != NAN) {
    delay(1);
  }
  else if (currentTempReadValue == NAN) {
    currentTempReadValue = thermocouple.readCelsius();
  }

  setPoint = myNex.readNumber("page1.n0.val");
  if (setPoint != 777777) {
    delay(1);
  }
  else if (setPoint == 777777) {
    setPoint = myNex.readNumber("page1.n0.val");
  }

  offsetTemp = myNex.readNumber("page1.n1.val");
  if (offsetTemp != 777777) {
    delay(1);
  }
  else if (offsetTemp == 777777) {
    offsetTemp = myNex.readNumber("page1.n1.val");
  }

  brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  if (brewTimeDelayTemp != 777777) {
    delay(1);
  }
  else if (brewTimeDelayTemp == 777777) {
    brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  }

  brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
  if (brewTimeDelayDivider != 777777) {
    delay(1);
  }
  else if (brewTimeDelayDivider == 777777) {
    brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
  }
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
  float currentTempReadValue = 0;
  int setPoint = 0;
  int offsetTemp = 0;
  int brewTimeDelayTemp = 0;
  int brewTimeDelayDivider = 0;

  // Making sure the serial communication finishes sending all the values
  currentTempReadValue = thermocouple.readCelsius();
  if (currentTempReadValue != NAN) {
    delay(1);
  }
  else if (currentTempReadValue == NAN) {
    currentTempReadValue = thermocouple.readCelsius();
  }

  setPoint = myNex.readNumber("page1.n0.val");
  if (setPoint != 777777) {
    delay(1);
  }
  else if (setPoint == 777777) {
    setPoint = myNex.readNumber("page1.n0.val");
  }

  offsetTemp = myNex.readNumber("page1.n1.val");
  if (offsetTemp != 777777) {
    delay(1);
  }
  else if (offsetTemp == 777777) {
    offsetTemp = myNex.readNumber("page1.n1.val");
  }

  brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  if (brewTimeDelayTemp != 777777) {
    delay(1);
  }
  else if (brewTimeDelayTemp == 777777) {
    brewTimeDelayTemp = myNex.readNumber("page2.n0.val");
  }

  brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
  if (brewTimeDelayDivider != 777777) {
    delay(1);
  }
  else if (brewTimeDelayDivider == 777777) {
    brewTimeDelayDivider = myNex.readNumber("page2.n1.val");
  }
  
  if (currentTempReadValue < float(setPoint+5)) {
    // Calculating the boiler heating power to apply
    int powerOutput = map(currentTempReadValue, setPoint-10, setPoint, brewTimeDelayTemp, brewTimeDelayTemp/brewTimeDelayDivider);
    if (powerOutput > brewTimeDelayTemp) {
      hPwrPrint = brewTimeDelayTemp;
      myNex.writeStr("page0.t1.txt", hPwrPrint);
      delay(10); //Small delay to hopefully get full serial communication
    }
    else if (powerOutput < brewTimeDelayTemp/brewTimeDelayDivider) {
      hPwrPrint = brewTimeDelayTemp/brewTimeDelayDivider;
      myNex.writeStr("page0.t1.txt", hPwrPrint);
      delay(10); //Small delay to hopefully get full serial communication
    }
    else {
      hPwrPrint = powerOutput;
      myNex.writeStr("page0.t1.txt", hPwrPrint);
      delay(10); //Small delay to hopefully get full serial communication
    }
  }
  else {
    waterTempPrint = currentTempReadValue - float(offsetTemp);
    myNex.writeStr("page0.t1.txt", "STEAM");
    delay(10); //Small delay to hopefully get full serial communication
  }
  //Printing the current values to the display
  waterTempPrint = currentTempReadValue - float(offsetTemp);
  myNex.writeStr("page0.t0.txt", waterTempPrint);
  delay(10); //Small delay to hopefully get full serial communication
}
