#include <Arduino.h>

#include "wnet_wifi.h"

void setup() {
  wnetMessageSetup();
}

void loop() {
  wnetMessageLoop();
}
