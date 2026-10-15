# ESP32 Wi-Fi messaging (Bài 1.6)

Two ESP32 boards exchange Serial messages while connected to the same Wi-Fi AP.
`main.cpp` runs `wnetMessageSetup()` and `wnetMessageLoop()` on both boards.

1. Set the station `ssid` and `password` in `src/wnet_wifi.cpp` for your 2.4 GHz AP.
   Use `nullptr` for an open network.
2. Configure the static addresses, gateway, subnet, and DNS in `src/wnet_wifi.cpp`
   to match your AP. The current settings use gateway/DNS `10.11.184.1` and
   subnet mask `255.255.248.0`:

   | Board | `isBoardA` | Local IP | Peer IP |
   | --- | --- | --- | --- |
   | A | `true` | `10.11.185.100` | `10.11.185.101` |
   | B | `false` | `10.11.185.101` | `10.11.185.100` |

   Choose two unused addresses allocated for these boards, outside the AP's DHCP
   pool or excluded from it, to avoid address conflicts.
3. Set `isBoardA = true` and upload to board A. Then set `isBoardA = false` and
   upload to board B. Select the corresponding serial port for each upload.
4. Open a Serial Monitor for each board at **115200 baud**, with **Newline** or
   **Both CR & LF** enabled. Wait for both boards to print their static addresses
   and peer addresses.
5. Send `Hello from ESP A` on the first board. It prints `[TX] ...`; the second
   board prints `[RX <sender IP>] Hello from ESP A`. Send a reply from the second
   board to check the reverse direction.

Messages use UDP unicast to the configured peer on port **4210**. Both boards
use static IP configuration instead of DHCP. Received packets are accepted only
from the configured peer IP and port; this filter does not authenticate the sender.
Each line is limited to **240 bytes**; longer lines are discarded.
UDP has no delivery acknowledgement: `[TX]` means the packet was submitted, and
the peer's `[RX]` confirms receipt. Messages are plain text and visible to other
listeners on the LAN.

The AP must allow UDP communication between clients. If an office
or guest network isolates devices, use a private router or hotspot with client
isolation disabled. Both boards must be on the same subnet.
