#ifndef ARENA_SIZE
#define ARENA_SIZE 65536
#endif // ARENA_SIZE

#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct arena_allocator {
  uint8_t*  arena_ptr;
  size_t current_block;
  size_t arena_size;
} arena_allocator;

/// Allocate a user defined amount of bytes using the allocator
void* arena_alloc(arena_allocator* allocator, size_t bytes);
void arena_clear (arena_allocator* allocator);
void arena_free  (arena_allocator* allocator);

#endif // ARENA_H

#ifdef ARENA_IMPLEMENTATION

#include <assert.h>

void* arena_alloc(arena_allocator* __restrict allocator, size_t bytes) {
  if (allocator->arena_size == 0) {
    allocator->arena_ptr  = malloc(ARENA_SIZE);
    allocator->arena_size = ARENA_SIZE;

    assert(allocator->arena_ptr != NULL);
  }

  if (bytes + allocator->current_block > allocator->arena_size) {
    return NULL;
  }

  void* current_block_ptr  =  allocator->arena_ptr + allocator->current_block;
  allocator->current_block += bytes;
  return current_block_ptr;
}

inline void arena_clear(arena_allocator* __restrict allocator) {
  allocator->current_block = 0;
}

inline void arena_free(arena_allocator* __restrict allocator) {
  free(allocator->arena_ptr);
}

#endif // ARENA_IMPLEMENTATION
