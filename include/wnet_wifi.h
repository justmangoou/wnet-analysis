#pragma once

// Station mode: connect to an access point
void wnetStationSetup(bool dhcp);
void wnetStationLoop();

// Two-way Serial messages with a configured peer on the same AP (static IP, UDP unicast).
void wnetMessageSetup();
void wnetMessageLoop();

// Access point mode: broadcast our own network
void wnetApSetup();
void wnetApLoop();
