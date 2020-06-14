#pragma once

#include <stdint.h>
#define BUFFER_SIZE 4 * 1024

#define DATA_TYPE_SVF  0
#define DATA_TYPE_XSVF 1

// External function to fetch next block to play
extern int fetch_next_block(uint8_t *buffer, int length);

int jtag_program(int dataType);
void set_pins(uint8_t tdi, uint8_t tdo, uint8_t tck, uint8_t tms, uint8_t led);
uint32_t jtag_chip_id();