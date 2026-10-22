#pragma once

// Change names to distinguish your boards from others in the lab.
static const char* BLUETOOTH_NAME = "WNET-ESP32-BT";
static const char* BLE_SERVER_NAME = "WNET-BLE-Server";
static const char* BLE_CLIENT_NAME = "WNET-BLE-Client";
static const char* BLE_SERVICE_UUID = "6b8a0001-3c6d-4fa0-9e21-7a1081f00101";
static const char* BLE_MESSAGE_UUID = "6b8a0002-3c6d-4fa0-9e21-7a1081f00101";

// Exercise 1: Classic Bluetooth Serial <-> USB Serial.
void bluetoothSetup();
void bluetoothLoop();

// Exercise 2: use the server pair on one ESP, the client pair on another.
void bluetoothBleServerSetup();
void bluetoothBleServerLoop();
void bluetoothBleClientSetup();
void bluetoothBleClientLoop();
