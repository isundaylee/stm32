#include "SPI.h"

#include <Utils.h>

SPI SPI_1(SPI1);
SPI SPI_2(SPI2);
SPI SPI_3(SPI3);
SPI SPI_4(SPI4);

void SPI::enableMaster(ClockPolarity cpol, ClockPhase cpha, DataFrameFormat dff,
                       BaudRate baud, NSSMode nssMode) {
  if (spi_ == SPI1) {
    BIT_SET(RCC->APB2ENR, RCC_APB2ENR_SPI1EN);
  } else if (spi_ == SPI2) {
    BIT_SET(RCC->APB1ENR, RCC_APB1ENR_SPI2EN);
  } else if (spi_ == SPI3) {
    BIT_SET(RCC->APB1ENR, RCC_APB1ENR_SPI3EN);
  } else if (spi_ == SPI4) {
    BIT_SET(RCC->APB2ENR, RCC_APB2ENR_SPI4EN);
  }

  // Frequency setting
  FIELD_SET(spi_->CR1, SPI_CR1_BR, E2I(baud));

  // Clock polarity and phase setting
  FIELD_SET(spi_->CR1, SPI_CR1_CPOL, E2I(cpol));
  FIELD_SET(spi_->CR1, SPI_CR1_CPHA, E2I(cpha));

  BIT_SET(spi_->CR1, SPI_CR1_MSTR);

  if (nssMode == NSSMode::AUTOMATIC) {
    BIT_CLEAR(spi_->CR1, SPI_CR1_SSM);
    BIT_SET(spi_->CR2, SPI_CR2_SSOE);
  } else if (nssMode == NSSMode::MANUAL) {
    BIT_SET(spi_->CR1, SPI_CR1_SSM);
    BIT_SET(spi_->CR1, SPI_CR1_SSI);
    BIT_CLEAR(spi_->CR2, SPI_CR2_SSOE);
  } else {
    // TODO: Error handling?
  }

  // Data frame format setting
  FIELD_SET(spi_->CR1, SPI_CR1_DFF, E2I(dff));

  // Enable it!
  BIT_SET(spi_->CR1, SPI_CR1_SPE);
}

void SPI::disable() {
  WAIT_UNTIL(BIT_IS_SET(spi_->SR, SPI_SR_TXE));
  WAIT_UNTIL(!BIT_IS_SET(spi_->SR, SPI_SR_BSY));

  BIT_CLEAR(spi_->CR1, SPI_CR1_SPE);
}

bool SPI::transmit(uint16_t const* data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    WAIT_UNTIL(BIT_IS_SET(spi_->SR, SPI_SR_TXE));
    spi_->DR = data[i];
  }

  WAIT_UNTIL(BIT_IS_SET(spi_->SR, SPI_SR_TXE));
  WAIT_UNTIL(!BIT_IS_SET(spi_->SR, SPI_SR_BSY));

  return true;
}
