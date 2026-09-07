#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE(arr) ((sizeof(arr)) / (sizeof(arr[0])))
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)

void die(const char *error_message) {
	printf("%s\n", error_message);
	exit(1);
}

#endif // !COMMON_H
