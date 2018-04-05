#pragma once

#include <DeviceHeader.h>

class ADC {
private:
  ADC_TypeDef *adc_;

public:
  ADC(ADC_TypeDef *adc) : adc_(adc) {}

  void enable();

  void selectChannel(int channel);

  uint16_t convert() {
    BIT_SET(adc_->CR2, ADC_CR2_SWSTART);
    WAIT_UNTIL(BIT_IS_SET(adc_->SR, ADC_SR_EOC));

    return adc_->DR;
  }

  void startContinuousConversion();
  void stopContinuousConversion();
  uint16_t getContinuousConversion() { return adc_->DR; }
};

extern ADC ADC_1;
extern ADC ADC_2;
extern ADC ADC_3;
