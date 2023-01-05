#include <Arduino.h>
#include "SerialTransfer.h"
#include "mcu_comms.h"

#define UART_MCU Serial1

McuComms comms;

Phase phaseArray[12];
Profile profile{ 6,  phaseArray };

void setup(void) {
  Serial.begin(115200);

  UART_MCU.setRxBufferSize(256);
  UART_MCU.setTxBufferSize(256);
  UART_MCU.begin(115200);
  // comms.setDebugPort(&Serial);
  comms.begin(UART_MCU);

  comms.setShotSnapshotCallback([](ShotSnapshot& snapshot) {
      Serial.print("Received shot snapshot {");
      Serial.print("time:"); Serial.print(snapshot.timeInShot);
      Serial.print(", pressure:"); Serial.print(snapshot.pressure);
      Serial.print(", flow:"); Serial.print(snapshot.flow);
      Serial.print(", temp:"); Serial.print(snapshot.temperature);
      Serial.print(", shotWeight:"); Serial.print(snapshot.shotWeight);
      Serial.print(", waterPumped:"); Serial.print(snapshot.waterPumped);
      Serial.println("}");
    }
  );
}

long profileTimer = -4000;
long snapshotTimer = 400;

void loop(void) {
  comms.readData();
  if (millis() - profileTimer > 100) {
    profile.count = 12;
    profile.globalStopConditions = GlobalStopConditions { .time=1923, .weight=37.f, .waterPumped=60.f};
    profile.phases[0] = Phase{ PHASE_TYPE_FLOW, Transition(4.5f), 2.f, PhaseStopConditions{ .pressureBelow=2.f } };
    profile.phases[1] = Phase{ PHASE_TYPE_FLOW, Transition(0.f), 2.f, PhaseStopConditions{ .time=3000 } };
    profile.phases[2] = Phase{ PHASE_TYPE_PRESSURE, Transition(9.f), -1, PhaseStopConditions{ .time=30000 } };
    profile.phases[3] = Phase{ PHASE_TYPE_PRESSURE, Transition(9.f, 7.f), 2.f, PhaseStopConditions{ .weight=35.f } };
    profile.phases[4] = Phase{ PHASE_TYPE_PRESSURE, Transition(7.f), 2.f, PhaseStopConditions{ .weight=35.f } };
    profile.phases[5] = Phase{ PHASE_TYPE_PRESSURE, Transition(7.f), 2.f, PhaseStopConditions{ .weight=36.f } };
    profile.phases[6] = Phase{ PHASE_TYPE_PRESSURE, Transition(7.f), 2.f, PhaseStopConditions{ .weight=37.f } };
    profile.phases[7] = Phase{ PHASE_TYPE_PRESSURE, Transition(7.f), 2.f, PhaseStopConditions{ .weight=38.f } };
    profile.phases[8] = Phase{ PHASE_TYPE_PRESSURE, Transition(7.f), 2.f, PhaseStopConditions{ .weight=39.f } };
    profile.phases[9] = Phase{ PHASE_TYPE_PRESSURE, Transition(7.f), 2.f, PhaseStopConditions{ .weight=40.f } };
    profile.phases[10] = Phase{ PHASE_TYPE_PRESSURE, Transition(7.f), 2.f, PhaseStopConditions{ .weight=41.f } };
    profile.phases[11] = Phase{ PHASE_TYPE_PRESSURE, Transition(7.f), 2.f, PhaseStopConditions{ .weight=42.f } };

    comms.sendProfile(profile);

    profileTimer = millis();
  }
}
