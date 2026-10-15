#include "wnet_wifi.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Station

static const char* ssid = "UET-Wifi-Office-Free 5hz";
static const char* password = NULL;

static IPAddress ip(10, 11, 185, 100);
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

void wnetStationSetup() {
  Serial.begin(115200);
  WiFi.config(ip, gateway, subnet, dns);
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
