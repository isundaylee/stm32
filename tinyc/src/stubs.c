#include <stddef.h>

// TODO: LOL
void* malloc(size_t size) {
  (void)size;
  return NULL;
}

void free(void* ptr) { (void)ptr; }

int memcmp(const void* a, const void* b, size_t n) {
  (void)a;
  (void)b;
  (void)n;
  return 0;
}
