# ESP32 Wi-Fi messaging (Bài 1.6)

Two ESP32 boards exchange Serial messages while connected to the same Wi-Fi AP.
`main.cpp` runs `wnetMessageSetup()` and `wnetMessageLoop()` on both boards.

1. Set the station `ssid` and `password` in `src/wnet_wifi.cpp` for your 2.4 GHz AP.
   Use `nullptr` for an open network.
2. Build and upload the same firmware to each ESP32 with PlatformIO. Select each
   board's serial port when uploading.
3. Open a Serial Monitor for each board at **115200 baud**, with **Newline** or
   **Both CR & LF** enabled. Wait for both boards to print their DHCP addresses.
4. Send `Hello from ESP A` on the first board. It prints `[TX] ...`; the second
   board prints `[RX <sender IP>] Hello from ESP A`. Send a reply from the second
   board to check the reverse direction.

Messages use subnet broadcast on UDP port **4210**, so no peer IP configuration
is needed. Each line is limited to **240 bytes**; longer lines are discarded.
UDP has no delivery acknowledgement: `[TX]` means the packet was submitted, and
the peer's `[RX]` confirms receipt. Messages are plain text and visible to other
listeners on the LAN.

The AP must allow communication between clients and UDP broadcasts. If an office
or guest network isolates devices, use a private router or hotspot with client
isolation disabled. Both boards must be on the same subnet.
