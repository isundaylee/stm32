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

int strcasecmp(const char* a, const char* b) {
  (void)a;
  (void)b;
  return 0;
}

int strcmp(const char* a, const char* b) {
  (void)a;
  (void)b;
  return 0;
}

char* strcpy(char* dest, const char* src) {
  (void)src;
  return dest;
}

size_t strlen(const char* str) {
  (void)str;
  return 0;
}
