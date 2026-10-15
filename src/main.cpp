#include <Arduino.h>

#include "wnet_wifi.h"

void setup() {
  Serial.begin(115200);
  wnetApSetup(true);
  wnetMessageSetup();
}

void loop() {
  wnetApLoop();
  wnetMessageLoop(true);
}
