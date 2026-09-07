#include <stdio.h>
#include <stdlib.h>

#include "common.h"

void die(const char *error_message) {
	printf("%s\n", error_message);
	exit(1);
}
