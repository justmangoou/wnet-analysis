#pragma once

// UART2: ESP32 TX -> A7680C RX, ESP32 RX <- A7680C TX.
static constexpr int MODEM_RX_PIN = 16;
static constexpr int MODEM_TX_PIN = 17;
static constexpr unsigned long MODEM_BAUD = 115200;

void cellularSetup();
void cellularLoop();
