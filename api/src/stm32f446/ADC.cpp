#include <ADC.h>
#include <USART.h>

void ADC::enable() {
  // Enables ADC clock
  if (adc_ == ADC1) {
    BIT_SET(RCC->APB2ENR, RCC_APB2ENR_ADC1EN);
  } else if (adc_ == ADC2) {
    BIT_SET(RCC->APB2ENR, RCC_APB2ENR_ADC2EN);
  } else if (adc_ == ADC3) {
    BIT_SET(RCC->APB2ENR, RCC_APB2ENR_ADC3EN);
  }

  // Enables ADC itself
  BIT_SET(adc_->CR2, ADC_CR2_ADON);

  // Conservative delay for tSTAB
  for (int i = 0; i < 100000; i++)
    asm volatile("nop");
}

void ADC::selectChannel(int channel) { adc_->SQR3 = channel; }

void ADC::startContinuousConversion() {
  BIT_CLEAR(adc_->CR2, ADC_CR2_SWSTART);
  BIT_SET(adc_->CR2, ADC_CR2_CONT);
  BIT_CLEAR(adc_->CR2, ADC_CR2_EOCS);

  // Force a read on DR so that EOC is cleared
  FORCE_READ(adc_->DR);

  BIT_SET(adc_->CR2, ADC_CR2_SWSTART);

  if (!BIT_IS_SET(adc_->CR2, ADC_CR2_DMA)) {
    WAIT_UNTIL(BIT_IS_SET(adc_->SR, ADC_SR_EOC));
  }
}

void ADC::stopContinuousConversion() {
  BIT_CLEAR(adc_->CR2, ADC_CR2_CONT);

  if (!BIT_IS_SET(adc_->CR2, ADC_CR2_DMA)) {
    WAIT_UNTIL(BIT_IS_SET(adc_->SR, ADC_SR_EOC));
  }
}

void ADC::enableDMA() {
  BIT_SET(adc_->CR2, ADC_CR2_DMA);
  BIT_SET(ADC1->CR2, ADC_CR2_DDS);
}

void ADC::disableDMA() {
  BIT_CLEAR(adc_->CR2, ADC_CR2_DMA);
  BIT_CLEAR(ADC1->CR2, ADC_CR2_DDS);
}

ADC ADC_1(ADC1);
ADC ADC_2(ADC2);
ADC ADC_3(ADC3);
