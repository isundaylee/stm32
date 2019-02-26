#include <Utils.h>

#include <stdlib.h>
#include <string.h>

#include <GPIO.h>
#include <USART.h>

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  GPIO_A.enable();
  GPIO_A.setMode(9, GPIO::PinMode::ALTERNATE, 7);  // TX
  GPIO_A.setMode(10, GPIO::PinMode::ALTERNATE, 7); // RX
  USART_1.enable(115200);

  char bufa[10] = "garbage";
  char const* cb;

  // strlen
  DEBUG_ASSERT(strlen("") == 0, "strlen test case 1 failed.");
  DEBUG_ASSERT(strlen("a") == 1, "strlen test case 2 failed.");
  DEBUG_ASSERT(strlen("bc\0") == 2, "strlen test case 3 failed.");

  // strcmp
  DEBUG_ASSERT(strcmp("hello", "hello") == 0, "strcmp test case 1 failed.");
  DEBUG_ASSERT(strcmp("1", "2") < 0, "strcmp test case 2 failed.");
  DEBUG_ASSERT(strcmp("2", "1") > 0, "strcmp test case 3 failed.");
  DEBUG_ASSERT(strcmp("1", "11") < 0, "strcmp test case 4 failed.");
  DEBUG_ASSERT(strcmp("11", "1") > 0, "strcmp test case 5 failed.");

  // strcpy
  cb = "hello";
  strcpy(bufa, cb);
  DEBUG_ASSERT(strcmp(bufa, cb) == 0, "strcpy test case 1 failed.");

  // strcasecmp
  DEBUG_ASSERT(strcasecmp("hello", "hELLO") == 0, "strcmp test case 1 failed.");
  DEBUG_ASSERT(strcasecmp("a", "B") < 0, "strcmp test case 2 failed.");
  DEBUG_ASSERT(strcasecmp("B", "a") > 0, "strcmp test case 3 failed.");
  DEBUG_ASSERT(strcasecmp("a", "AA") < 0, "strcmp test case 4 failed.");
  DEBUG_ASSERT(strcasecmp("BB", "b") > 0, "strcmp test case 5 failed.");

  // malloc
  void *pa = malloc(10), *pb = malloc(12), *pc = malloc(1 << 20);
  DEBUG_ASSERT(pa != NULL, "malloc test case 1 returned NULL");
  DEBUG_ASSERT(reinterpret_cast<uintptr_t>(pa) % 4 == 0,
               "malloc test case 1 not aligned");
  DEBUG_ASSERT(pb != NULL, "malloc test case 2 returned NULL");
  DEBUG_ASSERT(reinterpret_cast<uintptr_t>(pb) % 4 == 0,
               "malloc test case 2 not aligned");
  DEBUG_ASSERT(pc == NULL, "malloc test case 3 returned non-NULL");

  // memcmp
  uint8_t ma[] = {0x00, 0x00, 0x00};
  uint8_t mb[] = {0x00, 0x00, 0x01};
  DEBUG_ASSERT(memcmp(ma, mb, sizeof(ma)) < 0, "memcmp test case 1 failed.");
  DEBUG_ASSERT(memcmp(ma, ma, sizeof(ma)) == 0, "memcmp test case 2 failed.");
  DEBUG_ASSERT(memcmp(mb, ma, sizeof(ma)) > 0, "memcmp test case 3 failed.");

  // strncpy
  cb = "world";
  strncpy(bufa, cb, sizeof(bufa));
  DEBUG_ASSERT(strcmp(bufa, cb) == 0,
               "strncpy test case 1 failed assertion #1.");
  DEBUG_ASSERT(bufa[5] == '\0', "strncpy test case 1 failed assertion #2.");
  DEBUG_ASSERT(bufa[6] == '\0', "strncpy test case 1 failed assertion #3.");
  DEBUG_ASSERT(bufa[7] == '\0', "strncpy test case 1 failed assertion #4.");
  DEBUG_ASSERT(bufa[8] == '\0', "strncpy test case 1 failed assertion #5.");
  DEBUG_ASSERT(bufa[9] == '\0', "strncpy test case 1 failed assertion #6.");

  DEBUG_PRINT("All tests passed!\r\n");
}
