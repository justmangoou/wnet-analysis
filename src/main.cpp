#include <Arduino.h>

#include "wnet_wifi.h"

void setup() {
  Serial.begin(115200);
  wnetStationSetup(false);
  wnetMessageSetup();
}

void loop() {
  wnetMessageLoop();
}
