#include "GPIO.h"
#include "I2C.h"
#include "Clock.h"

#include "OLED.h"

#include "DigitBitmaps.h"

OLED<0x3C, 64, 128> oled(I2C_1);

size_t countdown = 10;

uint8_t const* digitToSprite(int digit) {
  switch (digit) {
  case 0:
    return BITMAP_CHAR_0;
  case 1:
    return BITMAP_CHAR_1;
  case 2:
    return BITMAP_CHAR_2;
  case 3:
    return BITMAP_CHAR_3;
  case 4:
    return BITMAP_CHAR_4;
  case 5:
    return BITMAP_CHAR_5;
  case 6:
    return BITMAP_CHAR_6;
  case 7:
    return BITMAP_CHAR_7;
  case 8:
    return BITMAP_CHAR_8;
  case 9:
    return BITMAP_CHAR_9;
  }

  return 0;
}

void updateCountdown() {
  size_t minutes = countdown / 60;
  size_t seconds = countdown % 60;

  oled.clearScreen();
  oled.sprite(12,  0, 40, 24, digitToSprite(minutes / 10));
  oled.sprite(12, 26, 40, 24, digitToSprite(minutes % 10));
  oled.sprite(12, 52, 40, 24, BITMAP_CHAR_COLON);
  oled.sprite(12, 78, 40, 24, digitToSprite(seconds / 10));
  oled.sprite(12, 104, 40, 24, digitToSprite(seconds % 10));

  oled.render();
}

void stop() {
  PWR->CR |= PWR_CR_CWUF;
  PWR->CR &= ~PWR_CR_PDDS;

  RCC->CFGR |= RCC_CFGR_STOPWUCK;
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  asm ("dsb");
  asm ("wfi");
}

void turnOLEDOn() {
  GPIO_A.set(1);

  for (int i=0; i<100000; i++)
    asm volatile ("nop");

  I2C_1.enable(I2C_SCL_Pin::I2C1_PA4, I2C_SDA_Pin::I2C1_PA10);

  oled.turnDisplayOff();
  oled.enableChargePump();
  oled.disableEntireDisplay();
  oled.turnDisplayOn();

  updateCountdown();
}

void turnOLEDOff() {
  GPIO_A.clear(1);

  for (int i=0; i<1000; i++) {
    asm volatile ("nop");
  }
}

extern "C" void main() {
  Clock::enableHSI();
  Clock::switchSysclk(Clock::Sysclk::HSI);

  GPIO_A.enable();
  GPIO_A.setMode(1, GPIO_MODE_OUTPUT);

  turnOLEDOn();

  while (true) {
    updateCountdown();

    if (countdown > 0) {
      countdown --;
    }

    if (countdown == 0) {
      turnOLEDOff();
      stop();
    }
  }
}
