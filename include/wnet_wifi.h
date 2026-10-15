#pragma once

#include <IPAddress.h>

// Station Config
// static const char* UPSTREAM_SSID = "UET-Wifi-Office-Free 2.4Ghz";
// static const char* UPSTREAM_PASSWORD = nullptr;
static const char* UPSTREAM_SSID = "Gateway_LAN";
static const char* UPSTREAM_PASSWORD = "12345678";

// Local station address and UDP destination. Swap these on the second board.
// static IPAddress ip = IPAddress(10, 11, 101, 115);
// static IPAddress peerIp = IPAddress(10, 11, 101, 116);
// static IPAddress dns(10, 11, 96, 1);
// static IPAddress gateway(10, 11, 96, 1);
// static IPAddress subnet(255, 255, 248, 0);
static IPAddress ip = IPAddress(192, 168, 4, 115);
static IPAddress peerIp = IPAddress(192, 168, 4, 116);
static IPAddress dns(192, 168, 4, 1);
static IPAddress gateway(192, 168, 4, 1);
static IPAddress subnet(255, 255, 255, 0);


// AP Config
static const char* AP_SSID = "Wifi Test";
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
void wnetMessageLoop();
