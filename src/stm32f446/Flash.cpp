#include <Flash.h>

/* static */ void Flash::setLatency(uint32_t latency) {
  FIELD_SET(FLASH->ACR, FLASH_ACR_LATENCY, latency);
  WAIT_UNTIL(FIELD_GET(FLASH->ACR, FLASH_ACR_LATENCY) == latency);
}
