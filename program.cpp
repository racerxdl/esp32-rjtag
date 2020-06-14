#include "program.h"
#include <Arduino.h>

uint8_t codeBuffer[BUFFER_SIZE];

extern "C" {
  int       xsvftool_esp_scan       (void);
  uint32_t  xsvftool_esp_id         (void);
  int       xsvftool_esp_program    (int (*file_getbyte)(), int x);
  int       xsvftool_esp_svf_packet (int (*packet_getbyte)(), int index, int final, char *report);
  void      xsvftool_esp_set_pins   (uint8_t tdi, uint8_t tdo, uint8_t tck, uint8_t tms);
}

uint8_t ledpin = 2;

struct buffer_state {
  int count;      // how many bytes in buffer
  int ptr;        // current reading pointer
  uint8_t blink;  // for the LED
} rd;

int get_next_byte() {
  if(rd.ptr >= rd.count) {
    // refill the buffer and update content
    rd.ptr = 0;
    rd.count = fetch_next_block(codeBuffer, BUFFER_SIZE);
    if(rd.count <= 0 || rd.count > BUFFER_SIZE) {
      return -1;
    }
    digitalWrite(ledpin, (rd.blink++) & 1);
  }

  return codeBuffer[rd.ptr++];
}

uint32_t jtag_chip_id() {
  xsvftool_esp_scan();
  return xsvftool_esp_id();
}

int jtag_program(int dataType) {
  int retval = -1;
  if (dataType != DATA_TYPE_SVF && dataType != DATA_TYPE_XSVF) {
    Serial.println("[JTAG] Invalid data type");
    return retval;
  }

  uint32_t chipId = xsvftool_esp_id();

  if (!chipId) {
    Serial.println("[JTAG] No devices found!");
    return retval;
  }

  Serial.print("[JTAG] Found device ");
  printf("%08x\n", chipId);

  Serial.println("[JTAG] Waiting first block");
  rd.ptr = 0;
  rd.count = fetch_next_block(codeBuffer, BUFFER_SIZE);

  if (rd.count <= 0) {
    Serial.println("[JTAG] No data available");
    return retval;
  }

  Serial.println("[JTAG] Programming...");

  pinMode(LED_BUILTIN, OUTPUT);
  retval = xsvftool_esp_program(get_next_byte, dataType);
  pinMode(LED_BUILTIN, INPUT);

  Serial.print("[JTAG] Programming finished with status ");
  Serial.println(retval);

  return retval;
}

void set_pins(uint8_t tdi, uint8_t tdo, uint8_t tck, uint8_t tms, uint8_t led) {
  xsvftool_esp_set_pins(tdi, tdo, tck, tms);
  ledpin = led;
}