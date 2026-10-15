#include <Arduino.h>

#include "wnet_wifi.h"

// Gateway: create the local AP and connect to the internet router.
void setup() {
  Serial.begin(115200);
  wnetApSetup(true);
  wnetMessageSetup();
}

void loop() {
  wnetApLoop();
  wnetMessageLoop(true);  // Forward received plain text to httpbin.
}
