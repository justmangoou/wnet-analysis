#pragma once

#include <IPAddress.h>

// Station Config
// static const char* UPSTREAM_SSID = "M-ESP-SERVER";
// static const char* UPSTREAM_PASSWORD = "12345678";
static const char* UPSTREAM_SSID = "UET-Wifi-Office-Free 2.4Ghz";
static const char* UPSTREAM_PASSWORD = nullptr;
// static const char* UPSTREAM_SSID = "YOUR_ROUTER_SSID";
// static const char* UPSTREAM_PASSWORD = "YOUR_ROUTER_PASSWORD";

// Client address and destination (gateway AP). Use .3 for the second client.
// static IPAddress ip = IPAddress(10, 11, 101, 115);
// static IPAddress peerIp = IPAddress(10, 11, 101, 116);
// static IPAddress dns(10, 11, 96, 1);
// static IPAddress gateway(10, 11, 96, 1);
// static IPAddress subnet(255, 255, 248, 0);
static IPAddress ip = IPAddress(192, 168, 10, 2);
static IPAddress peerIp = IPAddress(192, 168, 10, 1);
static IPAddress dns(192, 168, 10, 1);
static IPAddress gateway(192, 168, 10, 1);
static IPAddress subnet(255, 255, 255, 0);


// AP Config
static const char* AP_SSID = "M-ESP-SERVER";
static const char* AP_PASSWORD = "12345678";

static const int apChannel = 1;
static const int apMaxClients = 2;

static IPAddress apIp(192, 168, 10, 1);
static IPAddress apGateway(192, 168, 10, 1);
static IPAddress apSubnet(255, 255, 255, 0);

// Station mode: connect to an access point
void wnetStationSetup(bool dhcp);

// Access point mode: broadcast our own network
void wnetApSetup(bool withUpstream = false);
void wnetApLoop();

// Start Wi-Fi first, then UDP. Receives from all senders on the message port.
// Sends Serial input to peerIp, independently of station/AP/AP+STA mode.
void wnetMessageSetup();
// true forwards received text to httpbin; use only on the AP+STA gateway.
void wnetMessageLoop(bool uploadToHttpbin = false);
