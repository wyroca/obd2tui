#include <stdlib.h>
#include "arena.h"

arena *arena_create(size_t capacity) {
	arena *a = malloc(sizeof(arena));
	a->idx = 0;
	a->size = 0;
	a->capacity = capacity;
	a->data = malloc(capacity);
	return a;
}

void *arena_allocate(arena *a, size_t size) {
	if (a->idx + size > a->capacity - 1) {
		return NULL;
	}

	void *data = a->data + a->idx;
	a->idx += size;
	return data;
}
