#include "esp_comms.h"
#include "pindef.h"

McuComms mcuComms;

void espCommsInit() {
  USART_ESP.begin(115200);

  // mcuComms.setDebugPort(&USART_ESP);
  mcuComms.begin(USART_ESP);

  // Set callbacks
  mcuComms.setProfileReceivedCallback(onProfileReceived);
}

void espCommsReadData() {
    mcuComms.readData();
}

uint32_t sensorDataTimer = 0;
void espCommsSendSensorData(SensorState& state, bool brewActive, bool steamActive, uint32_t frequency) {
  uint32_t now = millis();
  if (now - sensorDataTimer > frequency) {
    SensorStateSnapshot sensorSnapshot = SensorStateSnapshot {
      .brewActive=brewActive,
      .steamActive=steamActive,
      .temperature=state.temperature,
      .pressure=state.smoothedPressure,
      .pumpFlow=state.smoothedPumpFlow,
      .weightFlow=state.weightFlow,
      .weight=state.weight,
    };
    mcuComms.sendSensorStateSnapshot(sensorSnapshot);
    sensorDataTimer = now;
  }
}

uint32_t shotDataTimer;
void espCommsSendShotData(ShotSnapshot& shotData, uint32_t frequency) {
    uint32_t now = millis();
  if (now - shotDataTimer > frequency) {
    mcuComms.sendShotData(shotData);
    shotDataTimer = now;
  }
}
