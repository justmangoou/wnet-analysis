#include "bluetooth.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <atomic>

// Server: accept text writes and make the last text available for readback.
static std::atomic<bool> restartAdvertising(false);

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) override {
    Serial.println("[BLE] Client connected");
  }
  void onDisconnect(BLEServer*) override {
    Serial.println("[BLE] Client disconnected");
    restartAdvertising = true;
  }
};

class MessageCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    std::string text = characteristic->getValue();
    Serial.print("[BLE RX] ");
    Serial.write(reinterpret_cast<const uint8_t*>(text.data()), text.size());
    Serial.println();
  }
};

static ServerCallbacks serverCallbacks;
static MessageCallbacks messageCallbacks;

void bluetoothBleServerSetup() {
  BLEDevice::init(BLE_SERVER_NAME);
  BLEServer* server = BLEDevice::createServer();
  server->setCallbacks(&serverCallbacks);
  BLEService* service = server->createService(BLE_SERVICE_UUID);
  BLECharacteristic* message = service->createCharacteristic(
      BLE_MESSAGE_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  message->setCallbacks(&messageCallbacks);
  message->setValue("");
  service->start();
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BLE_SERVICE_UUID);
  advertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.printf("[BLE] Server %s | MAC %s\n", BLE_SERVER_NAME,
                BLEDevice::getAddress().toString().c_str());
  Serial.println("[BLE] Advertising; waiting for client writes");
}

void bluetoothBleServerLoop() {
  if (restartAdvertising.exchange(false)) {
    delay(100);  // Let disconnection finish before advertising again.
    BLEDevice::startAdvertising();
    Serial.println("[BLE] Advertising restarted");
  }
  delay(10);
}

// Client: scan for the service, connect, then write Serial lines.
static BLEClient* bleClient = nullptr;
static BLERemoteCharacteristic* remoteMessage = nullptr;
static const size_t bleMaxLength = 20;  // Fits the default BLE ATT MTU.
static char bleLine[bleMaxLength + 1];
static size_t bleLength = 0;
static bool bleOverflow = false;
static bool wasConnected = false;

static bool connectToServer() {
  BLEScan* scan = BLEDevice::getScan();
  Serial.println("[BLE] Scanning for server service...");
  BLEScanResults results = scan->start(3, false);
  bool connected = false;
  for (int i = 0; i < results.getCount(); ++i) {
    BLEAdvertisedDevice device = results.getDevice(i);
    if (!device.isAdvertisingService(BLEUUID(BLE_SERVICE_UUID))) {
      continue;
    }
    Serial.printf("[BLE] Found %s\n", device.getAddress().toString().c_str());
    if (!bleClient->connect(&device)) {
      Serial.println("[BLE] Connection failed; will scan again");
      continue;
    }
    bleClient->getServices();  // Rediscover handles after a server restart.
    BLERemoteService* service = bleClient->getService(BLE_SERVICE_UUID);
    remoteMessage = service ? service->getCharacteristic(BLE_MESSAGE_UUID) : nullptr;
    if (remoteMessage && remoteMessage->canWrite() && remoteMessage->canRead()) {
      connected = true;
      break;
    }
    remoteMessage = nullptr;
    bleClient->disconnect();
    Serial.println("[BLE] Required readable/writable characteristic not found");
  }
  scan->clearResults();
  return connected;
}

void bluetoothBleClientSetup() {
  BLEDevice::init(BLE_CLIENT_NAME);
  bleClient = BLEDevice::createClient();
  BLEDevice::getScan()->setActiveScan(true);
  Serial.printf("[BLE] Client %s | MAC %s\n", BLE_CLIENT_NAME,
                BLEDevice::getAddress().toString().c_str());
  Serial.println("[BLE] Once connected, type up to 20 bytes and press Enter");
}

void bluetoothBleClientLoop() {
  if (!bleClient) {
    return;
  }
  if (!bleClient->isConnected()) {
    if (wasConnected) {
      Serial.println("[BLE] Disconnected; restarting scan");
      wasConnected = false;
    }
    remoteMessage = nullptr;
    bleLength = 0;
    bleOverflow = false;
    // Discard input entered while disconnected rather than send stale text later.
    while (Serial.available()) {
      Serial.read();
    }
    if (connectToServer()) {
      wasConnected = true;
      Serial.println("[BLE] Connected; type a message and press Enter");
    } else {
      delay(1000);
    }
    return;
  }
  for (size_t i = 0; i < 64 && Serial.available(); ++i) {
    char c = static_cast<char>(Serial.read());
    if (c == '\r' || c == '\n') {
      if (bleOverflow) {
        Serial.println("[BLE] More than 20 bytes; message discarded");
      } else if (bleLength > 0 && remoteMessage) {
        bleLine[bleLength] = '\0';
        std::string text(bleLine, bleLength);
        remoteMessage->writeValue(text, true);
        if (!bleClient->isConnected()) {
          bleLength = 0;
          bleOverflow = false;
          break;
        }
        std::string readback = remoteMessage->readValue();
        if (bleClient->isConnected() && readback == text) {
          Serial.printf("[BLE] Write verified: %s\n", bleLine);
        } else {
          Serial.println("[BLE] Readback did not confirm write; retry after reconnect");
        }
      }
      bleLength = 0;
      bleOverflow = false;
    } else if (!bleOverflow) {
      if (bleLength < bleMaxLength) {
        bleLine[bleLength++] = c;
      } else {
        bleOverflow = true;
      }
    }
  }
  delay(10);
}
