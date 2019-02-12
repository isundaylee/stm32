#include <Utils.h>

#if 1

const char* HexString(uint32_t n, size_t len /*=0*/) {
  static char buffer[11];
  static char* const bufferEnd = &buffer[sizeof(buffer) / sizeof(buffer[0])];

  char* p = bufferEnd - 1;

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

  if (len != 0) {
    while (static_cast<size_t>(bufferEnd - p) < (len + 2)) {
      *(p--) = '0';
    }
  }

  *(p--) = 'x';
  *(p--) = '0';

  return (p + 1);
}

const char* DecString(uint32_t n, size_t len /* = 0 */) {
  static char buffer[20];
  static char* const bufferEnd = &buffer[sizeof(buffer) / sizeof(buffer[0])];

  char* p = bufferEnd - 1;

  *(p--) = '\0';

  if (n != 0) {
    while (n != 0) {
      *(p--) = '0' + (n % 10);
      n /= 10;
    }
  } else {
    *(p--) = '0';
  }

  if (len != 0) {
    while (static_cast<size_t>(bufferEnd - p) < (len + 2)) {
      *(p--) = ' ';
    }
  }

  return (p + 1);
}

extern "C" void* memset(void* ptr, int value, size_t num) {
  for (size_t i = 0; i < num; i++) {
    static_cast<unsigned char*>(ptr)[i] = static_cast<unsigned char>(value);
  }

  return ptr;
}

#endif
