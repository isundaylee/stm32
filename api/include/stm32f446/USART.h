#pragma once

#include <DeviceHeader.h>
#include <RingBuffer.h>

const size_t USART_RX_BUFFER_SIZE = 32;

extern "C" void isrUSART1();
extern "C" void isrUSART2();
extern "C" void isrUSART3();
extern "C" void isrUSART6();

class USART {
private:
  USART_TypeDef* usart_;

  RingBuffer<uint8_t, USART_RX_BUFFER_SIZE> buffer_;

public:
  USART(USART_TypeDef* usart) : usart_(usart) {}

  void enable(uint32_t baudRate);

  void write(uint8_t data);
  void write(const char* data);
  void write(uint8_t* data, size_t length);

  void enableTxDMA();
  void disableTxDMA();

  int read();

  void flush();

  friend void isrUSART1();
  friend void isrUSART2();
  friend void isrUSART3();
  friend void isrUSART6();
};

extern USART USART_1;
extern USART USART_2;
extern USART USART_3;
extern USART USART_6;
