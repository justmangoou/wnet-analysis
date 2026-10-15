#include "wnet_wifi.h"

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

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

// Station

void wnetStationSetup(bool dhcp) {
  WiFi.mode(WIFI_STA);
  bool configured = dhcp
                        ? WiFi.config(IPAddress(), IPAddress(), IPAddress())
                        : WiFi.config(ip, gateway, subnet, dns);
  if (!configured) {
    Serial.println("Failed to configure station IP settings");
    return;
  }
  WiFi.setAutoReconnect(true);
  WiFi.begin(UPSTREAM_SSID, UPSTREAM_PASSWORD);

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

// AP

static uint32_t lastSignature = 0;
static bool apWithUpstream = false;
static IPAddress lastStationIp;

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

void wnetApSetup(bool withUpstream) {
  apWithUpstream = withUpstream;
  lastStationIp = IPAddress();
  WiFi.mode(withUpstream ? WIFI_AP_STA : WIFI_AP);
  WiFi.softAPConfig(apIp, apGateway, apSubnet);
  WiFi.softAP(AP_SSID, AP_PASSWORD, apChannel, 0, apMaxClients);
  if (withUpstream) {
    WiFi.config(IPAddress(), IPAddress(), IPAddress());
    WiFi.setAutoReconnect(true);
    WiFi.begin(UPSTREAM_SSID, UPSTREAM_PASSWORD);
  }

  Serial.println("AP started");
  Serial.print("SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP IP (gateway): ");
  Serial.println(WiFi.softAPIP());
  Serial.print("AP MAC: ");
  Serial.println(WiFi.softAPmacAddress());
}

void wnetApLoop() {
  if (apWithUpstream) {
    IPAddress stationIp = WiFi.status() == WL_CONNECTED
                              ? WiFi.localIP() : IPAddress();
    if (stationIp != lastStationIp) {
      lastStationIp = stationIp;
      if (stationIp != IPAddress()) {
        Serial.print("Station IP (upstream): ");
        Serial.println(stationIp);
      } else {
        Serial.println("Upstream station disconnected");
      }
    }
  }

  static uint32_t lastClientPoll = 0;
  if (millis() - lastClientPoll < 500) {
    return;
  }
  lastClientPoll = millis();
  uint32_t sig = printClients(false);
  if (sig != lastSignature) {
    lastSignature = sig;
    printClients(true);
  }
  // Keep the UDP receive loop responsive.
}
