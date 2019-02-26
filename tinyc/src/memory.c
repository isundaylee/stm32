#include <stddef.h>
#include <stdint.h>

#define HEAP_SIZE 64 * 1024

#define ALIGNMENT 4

static uint8_t __attribute__((aligned(4))) heap[HEAP_SIZE];
static uint8_t* heap_cur = heap;
static uint8_t* heap_top = heap + HEAP_SIZE;

typedef struct _node_t node_t;

struct _node_t {
  size_t size;

  union {
    struct {
      node_t* next;
    } free;

    struct {
    } allocated;
  } metadata;
};

node_t* freelist_head = NULL;

void* malloc(size_t size) {
  size += sizeof(node_t);
  size += (ALIGNMENT - (size % ALIGNMENT));

  // Try to allocate from the freelist first.
  for (node_t** cur = &freelist_head; *cur != NULL;
       cur = &(*cur)->metadata.free.next) {
    if ((*cur)->size >= size) {
      void* allocated = (void*)((uintptr_t)*cur + sizeof(node_t));
      *cur = (*cur)->metadata.free.next;
      return allocated;
    }
  }

  if (heap_cur + size > heap_top) {
    return NULL;
  }

  node_t* node = (node_t*)heap_cur;
  node->size = size;

  heap_cur += size;

  return (void*)((uintptr_t)node + sizeof(node_t));
}

void free(void* ptr) {
  node_t* node = (node_t*)((uintptr_t)ptr - sizeof(node_t));

  node->metadata.free.next = freelist_head;
  freelist_head = node;

  (void)ptr;
}

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
