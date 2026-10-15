#include <Arduino.h>

#include "wnet_wifi.h"

void setup() {
  wnetApSetup();
}

void loop() {
  wnetApLoop();
}
