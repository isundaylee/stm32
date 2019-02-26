#include <stddef.h>
#include <stdint.h>

#define HEAP_SIZE 64 * 1024

#define ALIGNMENT 4

uint8_t heap[HEAP_SIZE];
uint8_t* heap_cur = heap;
uint8_t* heap_top = heap + HEAP_SIZE;

void* malloc(size_t size) {
  if (heap_cur + size > heap_top) {
    return NULL;
  }

  size += (ALIGNMENT - (size % ALIGNMENT));

  void* allocated = (void*)heap_cur;
  heap_cur += size;

  return allocated;
}

void free(void* ptr) { (void)ptr; }
