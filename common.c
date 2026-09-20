#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "common.h"

void die(const char *error_message) {
	printf("%s\n", error_message);
	exit(1);
}

uint8_t hex_char_to_nibble(char c) {
	uint8_t result = 0;

	if (c >= 'A' && c <= 'F') {
		result += (c - 'A' + 10);
	} else if (c >= 'a' && c <= 'f') {
		result += (c - 'a' + 10);
	} else if (c >= '0' && c <= '9') {
		result += (c - '0');
	}

	return result;
}

uint8_t hex_chars_to_u8(const char *chars) {
	return 16 * hex_char_to_nibble(chars[0]) + hex_char_to_nibble(chars[1]);
}

uint32_t hex_chars_to_u32(const char *chars) {
	int shift = 28, idx = 0; 
	uint32_t result = 0;

	while (shift >= 0) {
		result += (hex_char_to_nibble(chars[idx])) << shift;
		idx++;
		if (chars[idx] == ' ') idx++;
		shift -= 4;
	}

	return result;
}
