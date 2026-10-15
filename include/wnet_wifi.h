#pragma once

// Station mode: connect to an access point
void wnetStationSetup(bool dhcp);
void wnetStationLoop();

// Two-way Serial messages between stations on the same AP (UDP broadcast).
void wnetMessageSetup();
void wnetMessageLoop();

// Access point mode: broadcast our own network
void wnetApSetup();
void wnetApLoop();
