#ifndef INCLUDE_WRITE_H
#define INCLUDE_WRITE_H

#include "common.h"
#include "kernel.h"

#define FB_COMMAND_PORT				0x3D4
#define FB_DATA_PORT					0x3D5
#define FB_HIGH_BYTE_COMMAND	14
#define FB_LOW_BYTE_COMMAND		15

void fb_clear(void);
void fb_put_char(char c);
void fb_write(char *buf);
void fb_write_dec(uint32_t n);
void fb_write_hex(uint32_t n);
void fb_init(uint8_t n);


#endif
