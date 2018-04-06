#pragma once

#include <DeviceHeader.h>

class DAC {
private:
  DAC_TypeDef *dac_;

public:
  DAC(DAC_TypeDef *dac) : dac_(dac) {}

  void enable();

  void enableChannel(int channel);
  void disableChannel(int channel);

  void setChannelValue(int channel, uint16_t value) {
    if (channel == 1) {
      dac_->DHR12R1 = value;
    } else {
      dac_->DHR12R2 = value;
    }
  }
};

extern DAC DAC_1;
