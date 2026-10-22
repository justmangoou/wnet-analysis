#include "cellular.h"

#include <Arduino.h>
#include <cstring>

static HardwareSerial modem(2);
static char inputLine[513];
static size_t inputLength = 0;
static bool overflow = false;
static bool previousCR = false;

static size_t txBytes = 0;
static size_t rxBytes = 0;
static bool probing = false;
static unsigned long probeStarted = 0;
static size_t probeRxStart = 0;
static char responseLine[96];
static size_t responseLength = 0;

static void writeModem(const uint8_t* data, size_t length) {
  size_t written = modem.write(data, length);
  modem.flush();  // Wait for UART transmission, not a modem acknowledgement.
  txBytes += written;
  Serial.printf("\n[4G TX] UART sent %u/%u bytes: ",
                static_cast<unsigned>(written), static_cast<unsigned>(length));
  for (size_t i = 0; i < written; ++i) {
    if (data[i] >= 32 && data[i] < 127) Serial.write(data[i]);
    else Serial.printf("<%02X>", data[i]);
  }
  Serial.println();
}

static void processResponse(char c) {
  if (!probing) return;
  if (c == '\r' || c == '\n') {
    responseLine[responseLength] = '\0';
    if (strcmp(responseLine, "OK") == 0 || strcmp(responseLine, "ERROR") == 0) {
      Serial.printf("\n[4G PROBE] %s after %lu ms\n",
                    responseLine, millis() - probeStarted);
      probing = false;
    }
    responseLength = 0;
  } else if (responseLength < sizeof(responseLine) - 1) {
    responseLine[responseLength++] = c;
  }
}

static void showHelp() {
  Serial.println("\n[4G] Enter AT commands; press Enter to send with CRLF.");
  Serial.println("[4G] /probe = send AT and wait 3 seconds for OK; /status = UART counters");
  Serial.println("[4G] /send = SMS Ctrl+Z, /cancel = SMS Escape, /help = help");
  Serial.println("[4G] Check: AT, ATI, AT+CPIN?, AT+CIMI, AT+COPS?, AT+CSQ");
  Serial.println("[4G] SMS: AT+CMGF=1, AT+CMGS=\"<number>\", wait for >,");
  Serial.println("[4G] then enter message, Enter, then /send and Enter.");
  Serial.println("[4G] Read: AT+CPMS?, AT+CMGR=<index>");
  Serial.println("[4G] Call: ATD<number>; | answer: ATA | hang up: ATH");
  Serial.println("[4G] Data: AT+CREG?, AT+CGATT?, AT+CGATT=1");
  Serial.println("[4G] Ping: AT+CPING=\"www.google.com\",1");
  Serial.println("[4G] TCP: AT+NETOPEN, AT+IPADDR");
  Serial.println("[4G] AT+CIPOPEN=1,\"TCP\",\"test.mosquitto.org\",1883");
  Serial.println("[4G] AT+CIPOPEN? lists sockets; TCP alone is not MQTT.");
}

static void sendLine() {
  inputLine[inputLength] = '\0';
  if (overflow) {
    Serial.println("\n[4G] Input exceeds 512 bytes; discarded");
  } else if (strcmp(inputLine, "/help") == 0) {
    showHelp();
  } else if (strcmp(inputLine, "/status") == 0) {
    Serial.printf("\n[4G] UART TX=%u RX=%u bytes; USB pending input=%u\n",
                  static_cast<unsigned>(txBytes), static_cast<unsigned>(rxBytes),
                  static_cast<unsigned>(inputLength));
  } else if (strcmp(inputLine, "/probe") == 0) {
    responseLength = 0;
    probeRxStart = rxBytes;
    probeStarted = millis();
    probing = true;
    const uint8_t command[] = {'A', 'T', '\r', '\n'};
    writeModem(command, sizeof(command));
  } else if (strcmp(inputLine, "/send") == 0) {
    const uint8_t end = 0x1a;
    writeModem(&end, 1);
    Serial.println("\n[4G] Sent SMS terminator (Ctrl+Z)");
  } else if (strcmp(inputLine, "/cancel") == 0) {
    const uint8_t escape = 0x1b;
    writeModem(&escape, 1);
    Serial.println("\n[4G] Sent Escape");
  } else {
    // Also permits empty lines in a multi-line SMS body.
    writeModem(reinterpret_cast<const uint8_t*>(inputLine), inputLength);
    const uint8_t ending[] = {'\r', '\n'};
    writeModem(ending, sizeof(ending));
  }
  inputLength = 0;
  overflow = false;
}

void cellularSetup() {
  modem.setRxBufferSize(2048);
  modem.begin(MODEM_BAUD, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);
  Serial.printf("\n[4G] A7680C UART2: RX=%d TX=%d baud=%lu\n",
                MODEM_RX_PIN, MODEM_TX_PIN, MODEM_BAUD);
  Serial.println("[4G] Power on the modem using its board's power procedure.");
  showHelp();
}

void cellularLoop() {
  // Preserve raw modem output, including SMS prompts and asynchronous events.
  for (size_t i = 0; i < 512 && modem.available(); ++i) {
    char c = static_cast<char>(modem.read());
    ++rxBytes;
    Serial.write(static_cast<uint8_t>(c));
    processResponse(c);
  }
  for (size_t i = 0; i < 128 && Serial.available(); ++i) {
    char c = static_cast<char>(Serial.read());
    if (c == '\n' && previousCR) {
      previousCR = false;
      continue;
    }
    previousCR = (c == '\r');
    if (c == '\r' || c == '\n') {
      sendLine();
    } else if (c == 0x1a || c == 0x1b) {
      if (overflow) {
        Serial.println("\n[4G] Oversize input discarded; use /cancel to cancel SMS");
      } else {
        writeModem(reinterpret_cast<const uint8_t*>(inputLine), inputLength);
        const uint8_t control = static_cast<uint8_t>(c);
        writeModem(&control, 1);
      }
      inputLength = 0;
      overflow = false;
    } else if (c == '\b' || c == 127) {
      if (!overflow && inputLength) --inputLength;
    } else if (!overflow) {
      if (inputLength < sizeof(inputLine) - 1) inputLine[inputLength++] = c;
      else overflow = true;
    }
  }
  if (probing && millis() - probeStarted >= 3000) {
    probing = false;
    Serial.printf("\n[4G PROBE] No OK within 3 seconds; received %u bytes.\n",
                  static_cast<unsigned>(rxBytes - probeRxStart));
    Serial.println("[4G] TX confirms ESP32 UART output only, not delivery to modem.");
    Serial.println("[4G] Zero RX: check modem power/PWRKEY, common GND, crossed TX/RX, baud.");
  }
  delay(1);
}
