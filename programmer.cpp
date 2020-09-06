#include "Arduino.h"
#include "programmer.h"
#include "program.h"
#include "storage.h"

uint8_t cmd = 0;
int32_t length = 0;

int passthrough = 0;
int lastled = 0;

void ReadCmdLength(uint8_t *cmd, int32_t *length) {
  while(Serial.available() < 5);

  *cmd = Serial.read();
  *length = 0;
  uint8_t *dataLengthbytes = (uint8_t*)length;

  // for (int i = 0; i < 4; i++) {
  for (int i = 3; i >= 0; i--) {
    dataLengthbytes[i] = Serial.read();
  }
  #ifdef DEBUG
  Serial.print("[INFO] CMD ");
  Serial.print((char) *cmd);
  Serial.print(" LENGTH ");
  Serial.println(*length);
  #endif
}

String ReadParam(int32_t length) {
  String param = "";
  for (int32_t i = 0; i < length; i++) {
    param += (char)Serial.read();
  }
  return param;
}

void ProgrammerInit() {
    set_pins(PIN_TDI, PIN_TDO, PIN_TCK, PIN_TMS, PIN_LED);

    pinMode(PIN_TDI, OUTPUT);
    pinMode(PIN_TDO, INPUT_PULLUP);
    pinMode(PIN_TCK, OUTPUT);
    pinMode(PIN_TMS, OUTPUT);
}

String tmpParam;

void ProgrammerLoop() {
  if (passthrough) {
    // Serial passthrough
    if(Serial.available()) {
      Serial2.write(Serial.read());
      digitalWrite(PIN_LED, (lastled++)&1);
    }

    if(Serial2.available()) {
      Serial.write(Serial2.read());
      digitalWrite(PIN_LED, (lastled++)&1);
    }

    return;
  }

  // Programming Mode
  if (Serial.available() >= 5) {
    ReadCmdLength(&cmd, &length);
    switch (cmd) {
      case CMD_START_SVF:
        jtag_program(DATA_TYPE_SVF, MODE_SERIAL);
        break;
      case CMD_START_XSVF:
        jtag_program(DATA_TYPE_XSVF, MODE_SERIAL);
        break;
      case CMD_QUERY:
        Serial.print("[QUERY] Chip ID: ");
        printf("%08x\n", jtag_chip_id());
        break;
      case CMD_STOP:
        Serial.println("[RDY]");
        break;
      case CMD_PASSTHROUGH:
        Serial.println("[PASS] Switching to Serial Passthrough");
        Serial.print("[PASS] Serial2 baudrate set to ");
        Serial.println(length);
        Serial2.begin(length);
        passthrough = 1;
        pinMode(PIN_LED, OUTPUT);
        digitalWrite(PIN_LED, LOW);
        Serial.println("[PASS] READY");
        return;
      case CMD_SET_WIFI_PASS:
        tmpParam = ReadParam(length);
        SaveWifiPassword(tmpParam);
        Serial.print("[INFO] Wifi Password set to ");
        Serial.println(tmpParam);
        tmpParam = "";
        break;
      case CMD_SET_WIFI_SSID:
        tmpParam = ReadParam(length);
        SaveWifiSSID(tmpParam);
        Serial.print("[INFO] Wifi SSID set to ");
        Serial.println(tmpParam);
        tmpParam = "";
        break;
      case CMD_SET_OTA_PASS:
        tmpParam = ReadParam(length);
        SaveOTAPassword(tmpParam);
        Serial.print("[INFO] OTA Password set to ");
        Serial.println(tmpParam);
        tmpParam = "";
        break;
      case CMD_SET_HOSTNAME:
        tmpParam = ReadParam(length);
        SaveHostname(tmpParam);
        Serial.print("[INFO] Hostname set to ");
        Serial.println(tmpParam);
        tmpParam = "";
        break;
      case CMD_REBOOT:
        Serial.println("[INFO] Received reboot");
        ESP.restart();
        break;
      default:
        Serial.println("[CTRL] ERROR");
    }
  }
}

int fetch_next_block(uint8_t *buffer, int length) {
  Serial.print("[CTRL] R");
  Serial.println(length);

  uint8_t cmd;
  int32_t dataLength;
  ReadCmdLength(&cmd, &dataLength);

  #ifdef DEBUG
  Serial.print("[CTRL] MReading ");
  Serial.print(dataLength);
  Serial.println(" bytes");
  #endif

  if (dataLength <= 0) {
    return -1;
  }

  int readBytes = 0;

  while (readBytes < dataLength) {
    while(!Serial.available());
    buffer[readBytes] = Serial.read();
    readBytes++;
  }
  Serial.println("[CTRL] A");

  return readBytes;
}
