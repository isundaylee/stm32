#include <Utils.h>

#include <GPIO.h>
#include <USART.h>

#if 1

constexpr USART& USART_DEBUG = USART_1;
constexpr uint32_t USART_BAUD_DEBUG = 115200;
constexpr GPIO::Pin PIN_DEBUG_0 = {&GPIO_C, 8};
constexpr GPIO::Pin PIN_DEBUG_1 = {&GPIO_C, 7};

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

extern "C" void* memcpy(void* __restrict dst, const void* __restrict src,
                        size_t num) {
  for (size_t i = 0; i < num; i++) {
    static_cast<unsigned char*>(dst)[i] =
        static_cast<const unsigned char*>(src)[i];
  }

  return dst;
}

extern "C" void __aeabi_memset(void* ptr, size_t num, int value) {
  // TODO: Register limit!!!!!!!!!!!!!
  memset(ptr, value, num);
}

extern "C" void __aeabi_memset4(void* ptr, size_t num, int value) {
  // TODO: Register limit!!!!!!!!!!!!!
  memset(ptr, value, num);
}

extern "C" void __aeabi_memclr(void* ptr, size_t num) {
  // TODO: Register limit!!!!!!!!!!!!!
  memset(ptr, 0, num);
}

extern "C" void _putchar(char ch) {
  USART_DEBUG.write(static_cast<uint8_t>(ch));
}

void debugInit() {
  GPIO_A.enable();
  GPIO_A.setMode(9, GPIO::PinMode::ALTERNATE, 7);  // TX
  GPIO_A.setMode(10, GPIO::PinMode::ALTERNATE, 7); // RX
  USART_1.enable(USART_BAUD_DEBUG);
}

void setDebugPin0() { PIN_DEBUG_0.gpio->set(PIN_DEBUG_0.pin); }
void clearDebugPin0() { PIN_DEBUG_0.gpio->clear(PIN_DEBUG_0.pin); }

void handleAssertionFailure(char const* message) {
  clearDebugPin0();
  printf("\r\nAssertion failure: %s\r\n", message);
  WAIT_UNTIL(false);
}

void setDebugPin1() { PIN_DEBUG_1.gpio->set(PIN_DEBUG_1.pin); }
void clearDebugPin1() { PIN_DEBUG_1.gpio->clear(PIN_DEBUG_1.pin); }

#endif
