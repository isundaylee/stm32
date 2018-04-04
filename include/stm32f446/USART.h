#pragma once

#include <DeviceHeader.h>

class USART {
private:
  USART_TypeDef *usart_;

public:
  USART(USART_TypeDef *usart) : usart_(usart) {}

  void enable();

  void write(uint8_t data);
  void write(const char* data);
  void write(uint8_t *data, size_t length);
};

extern USART USART_1;
extern USART USART_2;
extern USART USART_3;
extern USART USART_6;
