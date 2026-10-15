# Problem 3: plain-text UDP to HTTP

Two ESP32 clients send text over UDP to an ESP32 AP+STA gateway.
The gateway POSTs that exact text to `http://httpbin.org/post` with
`Content-Type: text/plain`. No JSON encoding, device roles, or background tasks.
httpbin responds with JSON, but the ESP only prints the HTTP status and sent text.

## Gateway

The checked-in `main.cpp` is the gateway program.
Set `UPSTREAM_SSID` and `UPSTREAM_PASSWORD` in `include/wnet_wifi.h` to your
internet router's 2.4 GHz network before uploading. Use `nullptr` for an open
network. The router's subnet must differ from the AP's 192.168.10.0/24 subnet.
Keep `AP_SSID` / `AP_PASSWORD` as the local LAN credentials.
The AP is 192.168.10.1; the gateway's upstream station uses DHCP.

## Clients

Replace `main.cpp` with this and upload to each client:

```cpp
#include <Arduino.h>
#include "wnet_wifi.h"

void setup() {
  Serial.begin(115200);
  wnetStationSetup(false);
  wnetMessageSetup();
}
void loop() {
  wnetMessageLoop();
}
```

Set each client's `UPSTREAM_SSID` / `UPSTREAM_PASSWORD` to match the gateway's
`AP_SSID` / `AP_PASSWORD` (default SSID: ESP_Server).

| Setting | Client 1 | Client 2 |
| --- | --- | --- |
| ip | 192.168.10.2 | 192.168.10.3 |
| peerIp | 192.168.10.1 | 192.168.10.1 |
| gateway / dns | 192.168.10.1 | 192.168.10.1 |
| subnet | 255.255.255.0 | 255.255.255.0 |

Upload each board with its own settings. Power the gateway first, then clients.
Open Serial Monitors at 115200 baud and wait for the gateway's upstream station
IP and both clients' Connected messages. Type `temperature=28` on either client
and press Enter. Gateway output should include:

```text
[RX 192.168.10.2] temperature=28
[HTTP] Uploaded: temperature=28 | status 200
```

Repeat from client 2 and confirm its source IP is 192.168.10.3.
These are manually entered readings; no physical sensor driver is included.

This deliberately simple version uploads synchronously: UDP processing pauses
during the HTTP request. Send readings slowly; it has no upload queue or retries.
If the upstream is offline, that reading is printed locally and the upload is
skipped. UDP has no delivery acknowledgement. Messages are at most 240 bytes.
HTTP is unencrypted for this demo. httpbin echoes requests but does not store
readings or provide a dashboard. AP clients do not need internet access or NAT.

For problem 2.4, use `wnetApSetup(false)` / `wnetApLoop()` on the AP and the
client program above on the messaging boards, with each `peerIp` set to the
other client's address. For problem 1.6, use the same client program with router
credentials and IP settings appropriate to that router.
