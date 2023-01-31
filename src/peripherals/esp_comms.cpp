#include "esp_comms.h"
#include "pindef.h"
namespace {
  class McuCommsSingleton {
  public:
    static McuComms& getInstance() {
      static McuComms instance;
      return instance;
    }
  private:
    McuCommsSingleton() = default;
    ~McuCommsSingleton() = default;
  };
}

void espCommsInit() {
  USART_ESP.begin(115200);

  // mcuComms.setDebugPort(&USART_ESP);
  McuCommsSingleton::getInstance().begin(USART_ESP);

  // Set callbacks
  McuCommsSingleton::getInstance().setProfileReceivedCallback(onProfileReceived);
}

void espCommsReadData() {
  McuCommsSingleton::getInstance().readData();
}

volatile uint32_t sensorDataTimer = 0;
void espCommsSendSensorData(const SensorState& state, bool brewActive, bool steamActive, uint32_t frequency) {
  uint32_t now = millis();
  if (now - sensorDataTimer > frequency) {
    SensorStateSnapshot sensorSnapshot = SensorStateSnapshot{
      .brewActive = brewActive,
      .steamActive = steamActive,
      .temperature = state.temperature,
      .pressure = state.smoothedPressure,
      .pumpFlow = state.smoothedPumpFlow,
      .weightFlow = state.weightFlow,
      .weight = state.weight,
    };
    McuCommsSingleton::getInstance().sendSensorStateSnapshot(sensorSnapshot);
    sensorDataTimer = now;
  }
}

volatile uint32_t shotDataTimer;
void espCommsSendShotData(ShotSnapshot& shotData, uint32_t frequency) {
  uint32_t now = millis();
  if (now - shotDataTimer > frequency) {
    McuCommsSingleton::getInstance().sendShotData(shotData);
    shotDataTimer = now;
  }
}
