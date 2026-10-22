#include "bluetooth.h"

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <atomic>

// Server: accept text writes and make the last text available for readback.
static std::atomic<bool> restartAdvertising(false);
static std::atomic<bool> phoneConnected(false);
static BLECharacteristic* uartTx = nullptr;
static uint8_t rxLine[254];
static size_t rxLength = 0;
static bool rxOverflow = false;
static bool rxWasCR = false;
static uint8_t txLine[256];
static size_t txLength = 0;
static bool txOverflow = false;
static bool txWasCR = false;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*) override {
    phoneConnected = true;
    Serial.println("[BLE] Client connected");
  }
  void onDisconnect(BLEServer*) override {
    rxLength = 0;
    rxOverflow = false;
    rxWasCR = false;
    phoneConnected = false;
    Serial.println("[BLE] Client disconnected");
    restartAdvertising = true;
  }
};

class MessageCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* characteristic) override {
    std::string text = characteristic->getValue();
    if (characteristic->getUUID().equals(BLEUUID(BLE_WRITE_UUID))) {
      // UART writes may contain single characters or fragments of a line.
      for (unsigned char c : text) {
        if (c == '\n' && rxWasCR) {
          rxWasCR = false;
          continue;
        }
        rxWasCR = (c == '\r');
        if (c == '\r' || c == '\n') {
          if (rxOverflow) {
            Serial.println("[BLE] RX line exceeds 254 bytes; discarded");
          } else {
            Serial.print("[BLE RX] ");
            Serial.write(rxLine, rxLength);
            Serial.println();
          }
          rxLength = 0;
          rxOverflow = false;
        } else if (c == '\b' || c == 127) {
          if (!rxOverflow && rxLength) --rxLength;
        } else if (!rxOverflow) {
          if (rxLength < sizeof(rxLine)) rxLine[rxLength++] = c;
          else rxOverflow = true;
        }
      }
      return;
    }
    // The exercise's custom characteristic still accepts standalone writes.
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
  BLECharacteristic* uartRx = service->createCharacteristic(
      BLE_WRITE_UUID, BLECharacteristic::PROPERTY_WRITE |
                          BLECharacteristic::PROPERTY_WRITE_NR);
  uartRx->setCallbacks(&messageCallbacks);
  uartTx = service->createCharacteristic(
      BLE_NOTIFY_UUID, BLECharacteristic::PROPERTY_READ |
                           BLECharacteristic::PROPERTY_NOTIFY);
  uartTx->addDescriptor(new BLE2902());
  uartTx->setValue("");
  service->start();
  Serial.printf("[BLE GATT] Firmware built %s %s\n", __DATE__, __TIME__);
  Serial.printf("[BLE GATT] Service %s | handle %u\n",
                BLE_SERVICE_UUID, service->getHandle());
  Serial.printf("[BLE GATT] Write   %s | handle %u\n",
                BLE_WRITE_UUID, uartRx->getHandle());
  Serial.printf("[BLE GATT] Notify  %s | handle %u\n",
                BLE_NOTIFY_UUID, uartTx->getHandle());
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
  for (size_t i = 0; i < 128 && Serial.available(); ++i) {
    char c = static_cast<char>(Serial.read());
    if (c == '\n' && txWasCR) {
      txWasCR = false;
      continue;
    }
    txWasCR = (c == '\r');
    if (c == '\r' || c == '\n') {
      if (txOverflow) {
        Serial.println("[BLE] TX line exceeds 254 bytes; discarded");
      } else if (!phoneConnected || !uartTx) {
        Serial.println("[BLE] No client connected; line not sent");
      } else {
        txLine[txLength++] = '\r';
        txLine[txLength++] = '\n';
        // Split only after Enter, to fit the default ATT payload size.
        size_t offset = 0;
        while (offset < txLength && phoneConnected) {
          size_t count = txLength - offset;
          if (count > 20) count = 20;
          uartTx->setValue(txLine + offset, count);
          uartTx->notify();
          offset += count;
          delay(10);
        }
        Serial.print("[BLE TX] ");
        Serial.write(txLine, offset);
        if (offset < txLength) {
          Serial.println("\n[BLE] Disconnected during send; line incomplete");
        }
      }
      txLength = 0;
      txOverflow = false;
    } else if (c == '\b' || c == 127) {
      if (!txOverflow && txLength) --txLength;
    } else if (!txOverflow) {
      if (txLength < sizeof(txLine) - 2) txLine[txLength++] = c;
      else txOverflow = true;
    }
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
