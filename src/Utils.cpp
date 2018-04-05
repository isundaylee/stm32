#include <Utils.h>

const char *HexString(uint32_t n) {
  static char buffer[10];

  buffer[9] = '\0';
  char *p = buffer + 8;

  if (n != 0) {
    for (; n != 0; n /= 16, p--) {
      if (n % 16 < 10) {
        *p = '0' + (n % 16);
      } else {
        *p = 'A' + ((n % 16) - 10);
      }
    }
  } else {
    *(p--) = '0';
  }
  
  *(p--) = 'x';
  *(p--) = '0';

  return (p + 1);
}
