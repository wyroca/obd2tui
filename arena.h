#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

typedef struct {
	void *data;
	size_t idx;
	size_t size;
	size_t capacity;
} arena;

#define ARENA_ALLOC(arena, type, num_elements) arena_allocate(arena, sizeof(type) * num_elements)

arena *arena_create(size_t capacity);
void *arena_allocate(arena *a, size_t size);


#endif // !ARENA_H
