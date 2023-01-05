#include <Arduino.h>
#include "SerialTransfer.h"
#include "mcu_comms.h"

#define UART_MCU Serial1

McuComms comms;

Phase phaseArray[8] = {
  Phase{}, Phase{}, Phase{}, Phase{}, Phase{}, Phase{}, Phase{}, Phase{}
};
Profile profile{ 6,  phaseArray };

void setup(void) {
  Serial.begin(115200);

  // comms.setDebugPort(&Serial);
  UART_MCU.begin(115200);
  comms.begin(UART_MCU);

  comms.setProfileReceivedCallback([](Profile& newProfile) {
    Serial.printf("Received profile {count: %d}\n", newProfile.count);
    for (int i = 0; i < newProfile.count; i++) {
      Serial.printf("phase[%d] {type: %d, transition:{", i, newProfile.phases[i].type);
      Serial.print(newProfile.phases[i].target.start);
      Serial.print("->");
      Serial.print(newProfile.phases[i].target.end);
      Serial.print("} restriction:");
      Serial.print(newProfile.phases[i].restriction);
      Serial.print(", stopConditions: {");
      Serial.print("time:"); Serial.print(newProfile.phases[i].stopConditions.time);
      Serial.print(", pressureAbove:"); Serial.print(newProfile.phases[i].stopConditions.pressureAbove);
      Serial.print(", pressureBelow:"); Serial.print(newProfile.phases[i].stopConditions.pressureBelow);
      Serial.print(", weight:"); Serial.print(newProfile.phases[i].stopConditions.weight);
      Serial.print(", flowAbove:"); Serial.print(newProfile.phases[i].stopConditions.flowAbove);
      Serial.print(", flowBelow:"); Serial.print(newProfile.phases[i].stopConditions.flowBelow);
      Serial.print(", waterPumpedInPhase:"); Serial.print(newProfile.phases[i].stopConditions.waterPumpedInPhase);
      Serial.println("}}");
    }
    Serial.print("globalStopConditions: {");
    Serial.print("time:"); Serial.print(newProfile.globalStopConditions.time);
    Serial.print(", weight:"); Serial.print(newProfile.globalStopConditions.weight);
    Serial.print(", waterPumped:"); Serial.print(newProfile.globalStopConditions.waterPumped);
    Serial.println("}");
    delete[] newProfile.phases;
  });
}

long profileTimer = -4000;
long snapshotTimer = 400;

void loop(void) {
  comms.readData();
  if (millis() - snapshotTimer > 100) {
      ShotSnapshot snapshot = ShotSnapshot{
        millis(),
        7.f + random(0, 2) / 10.f,
        2.f + random(0, 5) / 10.f,
        90.f + random(20, 30) / 10.f,
        millis() / 100000.f,
        0.f
      };

    comms.sendShotData(snapshot);
    snapshotTimer = millis();
  }
}
