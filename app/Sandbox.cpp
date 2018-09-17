#include "Clock.h"
#include "GPIO.h"
#include "I2C.h"
#include "RealTimeClock.h"

#include "OLED.h"

#include "DigitBitmaps.h"

OLED<0x3C, 64, 128> oled(I2C_1);

enum class State {
  IDLE,
  COUNTING,
  FIRING,
};

State state = State::COUNTING;
bool isLEDOn = false;
size_t countdown = 10;

////////////////////////////////////////////////////////////////////////////////
// Rendering code
////////////////////////////////////////////////////////////////////////////////

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

void renderCountdown(size_t countdown) {
  size_t minutes = countdown / 60;
  size_t seconds = countdown % 60;

  oled.clearScreen();
  oled.sprite(12, 0, 40, 24, digitToSprite(minutes / 100));
  oled.sprite(12, 26, 40, 24, digitToSprite(minutes % 10));
  oled.sprite(12, 52, 40, 24, BITMAP_CHAR_COLON);
  oled.sprite(12, 78, 40, 24, digitToSprite(seconds / 10));
  oled.sprite(12, 104, 40, 24, digitToSprite(seconds % 10));

  oled.render();
}

void renderFiring(size_t countdown) {
  if (countdown % 2 == 0) {
    oled.clearScreen();
    oled.render();
  } else {
    renderCountdown(0);
  }
}

void rerender() {
  if (state == State::COUNTING) {
    renderCountdown(countdown);
  } else if (state == State::FIRING) {
    renderFiring(countdown);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Power management code
////////////////////////////////////////////////////////////////////////////////

void stop() {
  return;

  PWR->CR |= PWR_CR_CWUF;
  PWR->CR &= ~PWR_CR_PDDS;

  RCC->CFGR |= RCC_CFGR_STOPWUCK;
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  asm("dsb");
  asm("wfi");
}

////////////////////////////////////////////////////////////////////////////////
// OLED switching code
////////////////////////////////////////////////////////////////////////////////

void ensureOLEDOn() {
  if (isLEDOn) {
    return;
  }

  GPIO_A.set(1);

  for (int i = 0; i < 100000; i++)
    asm volatile("nop");

  I2C_1.enable(I2C_SCL_Pin::I2C1_PA4, I2C_SDA_Pin::I2C1_PA10);

  oled.turnDisplayOff();
  oled.enableChargePump();
  oled.disableEntireDisplay();
  oled.turnDisplayOn();

  isLEDOn = true;
}

void ensureOLEDOff() {
  if (!isLEDOn) {
    return;
  }

  GPIO_A.clear(1);

  for (int i = 0; i < 1000; i++) {
    asm volatile("nop");
  }

  isLEDOn = false;
}

////////////////////////////////////////////////////////////////////////////////
// ISRs
////////////////////////////////////////////////////////////////////////////////

void wakeupTimerHandler() {
  GPIO_A.toggle(9);

  switch (state) {
  case State::IDLE:
    break;
  case State::COUNTING:
    if (--countdown == 0) {
      state = State::FIRING;
      countdown = 10;
    }
    break;
  case State::FIRING:
    if (--countdown == 0) {
      state = State::IDLE;
    }
    break;
  }
}

void add1MinuteButtonHandler() { countdown += 1; }

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  Clock::enableHSI();
  Clock::enableLSI();
  Clock::switchSysclk(Clock::Sysclk::HSI);

  for (int i = 0; i < 5000000; i++) {
    asm volatile("nop");
  }

  GPIO_A.enable();
  GPIO_A.setMode(1, GPIO_MODE_OUTPUT);
  GPIO_A.setMode(9, GPIO_MODE_INPUT);
  GPIO_A.setPullDirection(9, GPIO::PullDirection::PULL_UP);
  GPIO_A.enableExternalInterrupt(9, GPIO::TriggerDirection::FALLING_EDGE,
                                 add1MinuteButtonHandler);

  RealTimeClock::enable(RealTimeClock::RTCClock::LSI);
  RealTimeClock::setupWakeupTimer(1, RealTimeClock::WakeupTimerClock::CK_SPRE,
                                  wakeupTimerHandler);

  while (true) {
    switch (state) {
    case State::IDLE:
      ensureOLEDOff();
      break;
    case State::COUNTING:
      ensureOLEDOn();
      rerender();
      break;
    case State::FIRING:
      ensureOLEDOn();
      rerender();
      break;
    }

    stop();
  }
}
