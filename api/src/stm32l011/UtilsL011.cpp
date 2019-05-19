#include <UtilsL011.h>

#include <stdlib.h>

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

extern "C" void __aeabi_memcpy(void* __restrict dst, const void* __restrict src,
                               size_t num) {
  memcpy(dst, src, num);
}

extern "C" void __aeabi_memcpy4(void* __restrict dst,
                                const void* __restrict src, size_t num) {
  memcpy(dst, src, num);
}
