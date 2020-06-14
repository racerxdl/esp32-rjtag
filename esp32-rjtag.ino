#include <WiFi.h>
#include "program.h"

// #define DEBUG

#define BAUD_RATE 921600

#define PIN_TDI 33
#define PIN_TDO 32
#define PIN_TCK 27
#define PIN_TMS 26
#define PIN_LED 2

#define CMD_START_SVF   'i'
#define CMD_START_XSVF  'x'
#define CMD_DATA        'd'
#define CMD_STOP        's'
#define CMD_QUERY       'q'
#define CMD_PASSTHROUGH 'p'



void read_cmd_length(uint8_t *cmd, int32_t *length) {
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

int fetch_next_block(uint8_t *buffer, int length) {
  Serial.print("[CTRL] R");
  Serial.println(length);

  uint8_t cmd;
  int32_t dataLength;
  read_cmd_length(&cmd, &dataLength);

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

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    set_pins(PIN_TDI, PIN_TDO, PIN_TCK, PIN_TMS, PIN_LED);

    pinMode(PIN_TDI, OUTPUT);
    pinMode(PIN_TDO, INPUT_PULLUP);
    pinMode(PIN_TCK, OUTPUT);
    pinMode(PIN_TMS, OUTPUT);

    Serial.println("[RDY]");
}

uint8_t cmd = 0;
int32_t length = 0;

int passthrough = 0;
int lastled = 0;

void loop() {
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

  if (Serial.available() >= 5) {
    read_cmd_length(&cmd, &length);
    switch (cmd) {
      case CMD_START_SVF:
        jtag_program(DATA_TYPE_SVF);
        break;
      case CMD_START_XSVF:
        jtag_program(DATA_TYPE_XSVF);
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
      default:
        Serial.println("[CTRL] ERROR");
    }
  }
}
