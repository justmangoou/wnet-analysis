#include "bluetooth.h"

#include <Arduino.h>
#include <BluetoothSerial.h>

static const char* btName = "WNET-ESP32-BT";

static BluetoothSerial btSerial;

void bluetoothSetup() {
  Serial.begin(115200);
  if (!btSerial.begin(btName)) {
    Serial.println("Bluetooth init failed");
    return;
  }
  Serial.print("Bluetooth started: ");
  Serial.println(btName);
  Serial.print("Bluetooth MAC: ");
  Serial.println(btSerial.getBtAddressString());
}

void bluetoothLoop() {
  while (btSerial.available()) {
    Serial.write(btSerial.read());
  }
  while (Serial.available()) {
    btSerial.write(Serial.read());
  }
  delay(10);
}
