#ifndef REMOTE_SCALE_H
#define REMOTE_SCALE_H

#include "remote_scales.h"
#include "esp_comms.h"

bool remoteScalesPresent = false;
float remoteScalesLatestWeight = 0.f;
uint32_t remoteScalesLastWeightTime = 0;

const uint16_t REMOTE_SCALES_TARE_DEBOUNCE = 500;
uint32_t lastRemoteScalesTare = 0;
void remoteScalesTare(void) {
  uint32_t now = millis();
  if (now - lastRemoteScalesTare < REMOTE_SCALES_TARE_DEBOUNCE) {
    return;
  }

  espCommsSendTareScalesCommand();
  lastRemoteScalesTare = millis();
}

Measurement remoteScalesGetWeight(void) {
  return Measurement{ .value = remoteScalesLatestWeight, .millis = remoteScalesLastWeightTime };
}

bool remoteScalesIsPresent(void) {
  return remoteScalesPresent && (millis() - remoteScalesLastWeightTime < 5000);
}

void onRemoteScalesWeightReceived(float weight) {
  remoteScalesPresent = true;
  remoteScalesLatestWeight = weight;
  remoteScalesLastWeightTime = millis();
}

void onRemoteScalesDisconnected() {
  remoteScalesPresent = false;
}

#endif
