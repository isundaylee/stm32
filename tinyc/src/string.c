#include <stddef.h>

static char char_to_lower(char const c) {
  if (c >= 'A' && c <= 'Z') {
    return c + ('a' - 'A');
  } else {
    return c;
  }
}

int strcasecmp(const char* a, const char* b) {
  for (; (*a != 0) || (*b != 0); a++, b++) {
    char la = char_to_lower(*a);
    char lb = char_to_lower(*b);

    if (la != lb) {
      return (la < lb ? -1 : 1);
    }
  }

  return 0;
}

int strcmp(const char* a, const char* b) {
  for (; (*a != 0) || (*b != 0); a++, b++) {
    if (*a != *b) {
      return (*a < *b ? -1 : 1);
    }
  }

  return 0;
}

char* strcpy(char* dest, const char* src) {
  char* cur = dest;

  for (; *src != '\0'; cur++, src++)
    *cur = *src;
  *cur = '\0';

  return dest;
}

size_t strlen(const char* str) {
  size_t len = 0;

  while (*(str++) != '\0')
    len++;

  return len;
}
