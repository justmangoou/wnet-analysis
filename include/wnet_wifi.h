#pragma once

// Station mode: connect to an access point
void wnetStationSetup(bool dhcp);
void wnetStationLoop();

// Access point mode: broadcast our own network
void wnetApSetup();
void wnetApLoop();
