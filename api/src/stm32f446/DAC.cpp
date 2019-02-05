#include <DAC.h>

void DAC::enable() { BIT_SET(RCC->APB1ENR, RCC_APB1ENR_DACEN); }

void DAC::enableChannel(int channel) {
  if (channel == 1) {
    BIT_SET(dac_->CR, DAC_CR_EN1);
    BIT_SET(dac_->CR, DAC_CR_BOFF1);
  } else {
    BIT_SET(dac_->CR, DAC_CR_EN2);
    BIT_SET(dac_->CR, DAC_CR_BOFF2);
  }
}

void DAC::disableChannel(int channel) {
  if (channel == 1) {
    BIT_CLEAR(dac_->CR, DAC_CR_EN1);
  } else {
    BIT_CLEAR(dac_->CR, DAC_CR_EN2);
  }
}

DAC DAC_1(DAC1);
