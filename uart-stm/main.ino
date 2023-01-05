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
      Serial.println("}");
    }
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
    Serial.println("Sent snapshot data");
    snapshotTimer = millis();
  }
}
