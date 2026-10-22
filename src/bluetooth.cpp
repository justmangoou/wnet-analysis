#include "bluetooth.h"

#include <Arduino.h>
#include <BluetoothSerial.h>
#include <atomic>
#include <esp_gap_bt_api.h>

static BluetoothSerial btSerial;
static std::atomic<bool> restoreConnectability(false);
static uint8_t serialLine[256];
static size_t serialLength = 0;
static bool serialOverflow = false;
static bool previousWasCR = false;

void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("\n[BT] Client connected!");
  } else if (event == ESP_SPP_CLOSE_EVT) {
    Serial.println("\n[BT] Client disconnected!");
    restoreConnectability = true;
  }
}

void bluetoothSetup() {
  btSerial.register_callback(btCallback);

  if (!btSerial.begin(BLUETOOTH_NAME)) {
    Serial.println("Bluetooth init failed");
    return;
  }
  Serial.print("Bluetooth started: ");
  Serial.println(BLUETOOTH_NAME);
  Serial.print("Bluetooth MAC: ");
  Serial.println(btSerial.getBtAddressString());
  Serial.println("Waiting for a client to connect over RFCOMM...");
  Serial.println("[BT] USB: type up to 254 bytes, then Enter (CR or LF) to send.");
}

void bluetoothLoop() {
  if (restoreConnectability.exchange(false) && !btSerial.hasClient()) {
    esp_err_t result = esp_bt_gap_set_scan_mode(
        ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
    if (result == ESP_OK) {
      Serial.println("[BT] Ready for reconnection over RFCOMM...");
    } else {
      Serial.printf("[BT] Could not restore discoverability: %d\n", result);
    }
  }

  uint8_t buffer[128];
  size_t length = 0;

  while (btSerial.available()) {
    length = 0;
    while (btSerial.available() && length < sizeof(buffer)) {
      buffer[length++] = btSerial.read();
    }
    Serial.print("[BT RX] ");
    Serial.write(buffer, length);
    Serial.println();
  }

  // Keep partial USB input across loop calls; send only on a line ending.
  for (size_t i = 0; i < 128 && Serial.available(); ++i) {
    char c = static_cast<char>(Serial.read());
    if (c == '\n' && previousWasCR) {
      previousWasCR = false;
      continue;
    }
    previousWasCR = (c == '\r');
    if (c == '\r' || c == '\n') {
      if (serialOverflow) {
        Serial.println("[BT] Line exceeds 254 bytes; discarded");
      } else if (!btSerial.hasClient()) {
        Serial.println("[BT] No RFCOMM client connected; line not sent");
      } else {
        serialLine[serialLength++] = '\r';
        serialLine[serialLength++] = '\n';
        size_t sent = btSerial.write(serialLine, serialLength);
        Serial.print("[BT TX] ");
        Serial.write(serialLine, sent);
        if (sent < serialLength) {
          Serial.println("\n[BT] Some data could not be sent");
        }
      }
      serialLength = 0;
      serialOverflow = false;
    } else if (c == '\b' || c == 127) {
      if (!serialOverflow && serialLength > 0) {
        --serialLength;
      }
    } else if (!serialOverflow) {
      if (serialLength < sizeof(serialLine) - 2) {
        serialLine[serialLength++] = static_cast<uint8_t>(c);
      } else {
        serialOverflow = true;
      }
    }
  }
}
