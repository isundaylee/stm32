#include <USART.h>

void USART::enable() {
  // Enable USART clock
  if (usart_ == USART1) {
    BIT_SET(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
  } else if (usart_ == USART2) {
    BIT_SET(RCC->APB1ENR, RCC_APB1ENR_USART2EN);
  } else if (usart_ == USART3) {
    BIT_SET(RCC->APB1ENR, RCC_APB1ENR_USART3EN);
  } else if (usart_ == USART6) {
    BIT_SET(RCC->APB2ENR, RCC_APB2ENR_USART6EN);
  }

  // Enable USART
  BIT_SET(usart_->CR1, USART_CR1_UE);

  // Use 8-bit words
  BIT_CLEAR(usart_->CR1, USART_CR1_M);

  // Use 1 stop bit
  FIELD_SET(usart_->CR2, USART_CR2_STOP, 0b00);

  // Set baud rate to 115200
  usart_->BRR = 139;

  // Enable transmission
  BIT_SET(usart_->CR1, USART_CR1_TE);
}

void USART::write(uint8_t data) {
  WAIT_UNTIL(BIT_IS_SET(usart_->SR, USART_SR_TXE));
  usart_->DR = data;
}

void USART::write(const char *data) {
  while (*data != '\0') {
    write(*(data++));
  }
}

void USART::write(uint8_t *data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    write(data[i]);
  }
}

USART USART_1(USART1);
USART USART_2(USART2);
USART USART_3(USART3);
USART USART_6(USART6);
