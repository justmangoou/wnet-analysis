#include <Arduino.h>
#include "bluetooth.h"

void setup() {
  Serial.begin(115200);
  bluetoothBleServerSetup();
}

void loop() {
  bluetoothBleServerLoop();
}
