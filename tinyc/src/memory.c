#include <stddef.h>
#include <stdint.h>

#define HEAP_SIZE 64 * 1024

#define ALIGNMENT 4

static uint8_t __attribute__((aligned(4))) heap[HEAP_SIZE];
static uint8_t* heap_cur = heap;
static uint8_t* heap_top = heap + HEAP_SIZE;

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

int memcmp(const void* a, const void* b, size_t n) {
  uint8_t* ua = (uint8_t*)a;
  uint8_t* ub = (uint8_t*)b;

  for (; n > 0; ua++, ub++, n--) {
    if (*ua != *ub) {
      return (*ua < *ub ? -1 : 1);
    }
  }

  return 0;
}
