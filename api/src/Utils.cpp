#include <Utils.h>

#if 1

const char *HexString(uint32_t n) {
  static char buffer[11];

  char *p = &buffer[sizeof(buffer) / sizeof(buffer[0]) - 1];

  *(p--) = '\0';

  if (n != 0) {
    while (n != 0) {
      if (n % 16 < 10) {
        *(p--) = '0' + (n % 16);
      } else {
        *(p--) = 'A' + ((n % 16) - 10);
      }
      n /= 16;
    }
  } else {
    *(p--) = '0';
  }

  *(p--) = 'x';
  *(p--) = '0';

  return (p + 1);
}

const char *DecString(uint32_t n) {
  static char buffer[20];

  char *p = &buffer[sizeof(buffer) / sizeof(buffer[0]) - 1];

  *(p--) = '\0';

  if (n != 0) {
    while (n != 0) {
      *(p--) = '0' + (n % 10);
      n /= 10;
    }
  } else {
    *(p--) = '0';
  }

  return (p + 1);
}

#endif
