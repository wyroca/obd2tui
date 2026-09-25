#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#define ARRAY_SIZE(arr) ((sizeof(arr)) / (sizeof(arr[0])))
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

void die(const char *error_message);

uint8_t hex_char_to_nibble(char c);
uint8_t hex_chars_to_u8(const char *chars);
uint16_t hex_chars_to_u16(const char *chars);
uint32_t hex_chars_to_u32(const char *chars);

void u8_to_hex_chars(char *dest_buf, uint8_t val);

#endif // !COMMON_H
