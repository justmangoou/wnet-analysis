#include <Arduino.h>

#include "wnet_wifi.h"

void setup() {
  wnetStationSetup(true);
}

void loop() {
  wnetStationLoop();
}
