#include "wnet_wifi.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>

static WiFiUDP messageUdp;
static const uint16_t messagePort = 4210;
static const size_t messageMaxLength = 240;
static char messageLine[messageMaxLength + 1];
static size_t messageLength = 0;
static bool messageOverflow = false;
static bool messageListening = false;
static uint32_t messageLastBindAttempt = 0;

// Messaging uses whichever Wi-Fi interfaces the application has already started.
static bool messageNetworkReady() {
  wifi_mode_t mode = WiFi.getMode();
  return WiFi.status() == WL_CONNECTED ||
         ((mode == WIFI_AP || mode == WIFI_AP_STA) &&
          WiFi.softAPIP() != IPAddress());
}

static void sendMessage() {
  if (!messageNetworkReady() || !messageListening) {
    Serial.println("Not connected; message was not sent");
    return;
  }

  if (!messageUdp.beginPacket(peerIp, messagePort)) {
    Serial.println("Failed to start message packet");
    return;
  }
  size_t written = messageUdp.write(
      reinterpret_cast<const uint8_t*>(messageLine), messageLength);
  int sent = messageUdp.endPacket();
  if (written != messageLength || !sent) {
    Serial.println("Failed to send message");
    return;
  }
  Serial.printf("[TX] %s\n", messageLine);
}

void wnetMessageSetup() {
  messageUdp.stop();
  messageListening = false;
  messageLength = 0;
  messageOverflow = false;
  messageLastBindAttempt = millis();
  messageListening = messageNetworkReady() && messageUdp.begin(messagePort) != 0;
  if (!messageListening) {
    Serial.println("Waiting for Wi-Fi / UDP listener; will retry");
  }
  Serial.printf("UDP peer: %s:%u\n", peerIp.toString().c_str(), messagePort);
  Serial.printf("Messages use UDP port %u. Type up to %u bytes and press Enter.\n",
                messagePort, static_cast<unsigned>(messageMaxLength));
}

// Plain HTTP for this exercise: send the received text unchanged.
static void uploadMessage(char* text, size_t length) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] No upstream connection; upload skipped");
    return;
  }
  WiFiClient client;
  HTTPClient http;
  http.setConnectTimeout(3000);
  http.setTimeout(5000);
  if (!http.begin(client, "http://httpbin.org/post")) {
    Serial.println("[HTTP] Failed to start request");
    return;
  }
  http.addHeader("Content-Type", "text/plain");
  int status = http.POST(reinterpret_cast<uint8_t*>(text), length);
  if (status >= 200 && status < 300) {
    Serial.printf("[HTTP] Uploaded: %s | status %d\n", text, status);
  } else if (status > 0) {
    Serial.printf("[HTTP] Upload failed: status %d\n", status);
  } else {
    Serial.printf("[HTTP] Upload failed: %s\n",
                  HTTPClient::errorToString(status).c_str());
  }
  http.end();
}

void wnetMessageLoop(bool uploadToHttpbin) {
  if (!messageNetworkReady()) {
    if (messageListening) {
      messageUdp.stop();
      messageListening = false;
    }
  } else if (!messageListening &&
             millis() - messageLastBindAttempt >= 1000) {
    messageLastBindAttempt = millis();
    messageListening = messageUdp.begin(messagePort) != 0;
  }

  if (messageListening) {
    int packetSize = messageUdp.parsePacket();
    if (packetSize > 0) {
      IPAddress sender = messageUdp.remoteIP();
      // Accept readings from multiple local ESPs, including in AP+STA mode.
      if (messageUdp.remotePort() == messagePort &&
          packetSize <= messageMaxLength) {
        char received[messageMaxLength + 1];
        int count = messageUdp.read(received, messageMaxLength);
        if (count > 0) {
          received[count] = '\0';
          Serial.printf("[RX %s] %s\n", sender.toString().c_str(), received);
          if (uploadToHttpbin) {
            uploadMessage(received, static_cast<size_t>(count));
          }
        }
      }
      messageUdp.flush();
    }
  }

  // Bound work per loop so incoming packets still get serviced while typing.
  for (size_t i = 0; i < 64 && Serial.available(); ++i) {
    char c = static_cast<char>(Serial.read());
    if (c == '\r' || c == '\n') {
      if (messageOverflow) {
        Serial.println("Message too long (maximum 240 bytes); discarded");
      } else if (messageLength > 0) {
        messageLine[messageLength] = '\0';
        sendMessage();
      }
      messageLength = 0;
      messageOverflow = false;
    } else if (!messageOverflow) {
      if (messageLength < messageMaxLength) {
        messageLine[messageLength++] = c;
      } else {
        messageOverflow = true;
      }
    }
  }
  delay(1);
}
