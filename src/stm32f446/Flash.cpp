#include <Flash.h>

/* static */ void Flash::setLatency(uint32_t latency) {
  FLASH->ACR &= ~(FLASH_ACR_LATENCY_Msk);
  FLASH->ACR |= (latency << FLASH_ACR_LATENCY_Pos);
}
