#include "wnet_wifi.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>

// Station

static const char* ssid = "UET-Wifi-Office-Free 2.4Ghz";
static const char* password = NULL;

// Set true for board A, false for board B before uploading.
static const bool isBoardA = true;
static IPAddress ip = isBoardA ? IPAddress(10, 11, 101, 115)
                             : IPAddress(10, 11, 101, 116);
static IPAddress peerIp = isBoardA ? IPAddress(10, 11, 101, 116)
                                 : IPAddress(10, 11, 101, 115);
static IPAddress dns(10, 11, 184, 1);
static IPAddress gateway(10, 11, 184, 1);
static IPAddress subnet(255, 255, 248, 0);

static void scanNetworks() {
  Serial.println("\nScanning...");
  int n = WiFi.scanNetworks();
  if (n <= 0) {
    Serial.println("No networks found");
    return;
  }
  Serial.printf("%d networks found\n", n);
  for (int i = 0; i < n; i++) {
    Serial.printf("%2d: %s | RSSI: %d dBm | Ch: %d | %s\n", i + 1,
                  WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i),
                  WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "Open" : "Secured");
  }
  WiFi.scanDelete();
}

void wnetStationSetup(bool dhcp) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  bool configured = dhcp
                        ? WiFi.config(IPAddress(), IPAddress(), IPAddress())
                        : WiFi.config(ip, gateway, subnet, dns);
  if (!configured) {
    Serial.println("Failed to configure station IP settings");
    return;
  }
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.print("BSSID (AP MAC): ");
  Serial.println(WiFi.BSSIDstr());
  Serial.print("ESP32 MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());

  scanNetworks();
}

void wnetStationLoop() {}

static WiFiUDP messageUdp;
static const uint16_t messagePort = 4210;
static const size_t messageMaxLength = 240;
static char messageLine[messageMaxLength + 1];
static size_t messageLength = 0;
static bool messageOverflow = false;
static bool messageListening = false;
static uint32_t messageLastBindAttempt = 0;

static void sendMessage() {
  if (WiFi.status() != WL_CONNECTED || !messageListening) {
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
  wnetStationSetup(false);
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  messageLastBindAttempt = millis();
  messageListening = messageUdp.begin(messagePort) != 0;
  if (!messageListening) {
    Serial.println("Failed to start UDP listener; will retry");
  }
  Serial.printf("Board %s | Peer: %s:%u\n", isBoardA ? "A" : "B",
                peerIp.toString().c_str(), messagePort);
  Serial.printf("Messages use UDP port %u. Type up to %u bytes and press Enter.\n",
                messagePort, static_cast<unsigned>(messageMaxLength));
}

void wnetMessageLoop() {
  if (WiFi.status() != WL_CONNECTED) {
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
      if (sender == peerIp && messageUdp.remotePort() == messagePort &&
          packetSize <= messageMaxLength) {
        char received[messageMaxLength + 1];
        int count = messageUdp.read(received, messageMaxLength);
        if (count > 0) {
          received[count] = '\0';
          Serial.printf("[RX %s] %s\n", sender.toString().c_str(), received);
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

// AP

static const char* apSsid = "Namper";
static const char* apPassword = "36363636";
static const int apChannel = 1;
static const int apMaxClients = 2;

static IPAddress apIp(192, 168, 10, 1);
static IPAddress apGateway(192, 168, 10, 1);
static IPAddress apSubnet(255, 255, 255, 0);

static uint32_t lastSignature = 0;

static uint32_t printClients(bool print) {
  wifi_sta_list_t list;
  tcpip_adapter_sta_list_t ipList;
  if (esp_wifi_ap_get_sta_list(&list) != ESP_OK ||
      tcpip_adapter_get_sta_list(&list, &ipList) != ESP_OK) {
    return 0;
  }
  uint32_t sig = list.num + 1;
  if (print) {
    Serial.printf("Connected devices: %d\n", list.num);
    if (list.num >= apMaxClients) {
      Serial.printf("Note: AP client limit reached (%d/%d). "
                    "New devices cannot connect until a client disconnects.\n",
                    list.num, apMaxClients);
    }
  }
  for (int i = 0; i < ipList.num; i++) {
    const uint8_t* m = ipList.sta[i].mac;
    IPAddress clientIp(ipList.sta[i].ip.addr);
    sig = sig * 31 + (uint32_t)ipList.sta[i].ip.addr + m[5];
    if (print) {
      Serial.printf("  %d: %02X:%02X:%02X:%02X:%02X:%02X  IP: %s\n", i + 1,
                    m[0], m[1], m[2], m[3], m[4], m[5],
                    clientIp.toString().c_str());
    }
  }
  return sig;
}

void wnetApSetup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIp, apGateway, apSubnet);
  WiFi.softAP(apSsid, apPassword, apChannel, 0, apMaxClients);

  Serial.println("AP started");
  Serial.print("SSID: ");
  Serial.println(apSsid);
  Serial.print("AP IP (gateway): ");
  Serial.println(WiFi.softAPIP());
  Serial.print("AP MAC: ");
  Serial.println(WiFi.softAPmacAddress());
}

void wnetApLoop() {
  uint32_t sig = printClients(false);
  if (sig != lastSignature) {
    lastSignature = sig;
    printClients(true);
  }
  delay(500);
}
