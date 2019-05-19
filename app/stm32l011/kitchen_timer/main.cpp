#include "Clock.h"
#include "GPIO.h"
#include "I2C.h"
#include "RealTimeClock.h"
#include "Tick.h"
#include "Timer.h"

#include "OLED.h"

#include "DigitBitmaps.h"

OLED<0x3C, 64, 128> oled(I2C_1);

enum class State {
  IDLE,
  COUNTING,
  FIRING,
};

bool isLEDOn = false;
State state = State::IDLE;
size_t countdown = 0;

static const uint32_t PIN_OLED_ON = 7;

static const uint32_t PIN_BUT_1 = 1;
static const uint32_t PIN_BUT_2 = 2;
static const uint32_t PIN_BUT_3 = 3;
static const uint32_t PIN_BUT_4 = 4;

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
  oled.sprite(12, 0, 40, 24, digitToSprite(minutes / 10));
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

  GPIO_A.set(PIN_OLED_ON);

  for (int i = 0; i < 100000; i++)
    asm volatile("nop");

  I2C_1.enable(I2C::SCLPin::I2C1_PA9, I2C::SDAPin::I2C1_PA10);

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

  GPIO_A.clear(PIN_OLED_ON);

  for (int i = 0; i < 1000; i++) {
    asm volatile("nop");
  }

  isLEDOn = false;
}

////////////////////////////////////////////////////////////////////////////////
// ISRs
////////////////////////////////////////////////////////////////////////////////

void timerHandler() {
  if (countdown % 2 == 0) {
    GPIO_C.toggle(14);
  } else {
    GPIO_C.clear(14);
  }
}

void wakeupTimerHandler() {
  switch (state) {
  case State::IDLE:
    break;
  case State::COUNTING:
    if (--countdown == 0) {
      state = State::FIRING;
      countdown = 10;
      Timer_2.startPeriodic(16, 1000, timerHandler);
    }
    break;
  case State::FIRING:
    if (--countdown == 0) {
      state = State::IDLE;
      Timer_2.stop();
    }
    break;
  }
}

static void addTime(size_t time) {
  switch (state) {
  case State::IDLE:
  case State::FIRING:
    countdown = time;
    state = State::COUNTING;
    Timer_2.stop();
    break;
  case State::COUNTING:
    countdown += time;
    break;
  }
}

void buttonHandler(size_t& lastPressedTick, bool& buttonPressed, size_t time) {
  size_t currentTick = Tick::value;

  if (currentTick - lastPressedTick < 5) {
    lastPressedTick = currentTick;
    return;
  }

  buttonPressed = !buttonPressed;
  if (buttonPressed) {
    addTime(time);
  }

  lastPressedTick = currentTick;
}

void button1Handler() {
  static size_t lastPressedTick = 0;
  static bool buttonPressed = false;
  buttonHandler(lastPressedTick, buttonPressed, 15);
}

void button2Handler() {
  static size_t lastPressedTick = 0;
  static bool buttonPressed = false;
  buttonHandler(lastPressedTick, buttonPressed, 60);
}

void button3Handler() {
  static size_t lastPressedTick = 0;
  static bool buttonPressed = false;
  buttonHandler(lastPressedTick, buttonPressed, 300);
}

void button4Handler() {
  static size_t lastPressedTick = 0;
  static bool buttonPressed = false;
  buttonHandler(lastPressedTick, buttonPressed, 600);
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  Clock::enableHSI();
  Clock::enableLSI();
  Clock::switchSysclk(Clock::Sysclk::HSI);

  GPIO_A.enable();
  GPIO_A.setMode(0, GPIO::PinMode::OUTPUT);

  GPIO_C.enable();
  GPIO_C.setMode(14, GPIO::PinMode::OUTPUT);

  Tick::enable();

  Timer_2.enable();

  for (int i = 0; i < 5000000; i++) {
    asm volatile("nop");
  }

  GPIO_A.setMode(PIN_OLED_ON, GPIO::PinMode::OUTPUT);

  GPIO_A.setMode(PIN_BUT_1, GPIO::PinMode::INPUT);
  GPIO_A.setPullDirection(PIN_BUT_1, GPIO::PullDirection::PULL_UP);
  GPIO_A.enableExternalInterrupt(PIN_BUT_1, GPIO::TriggerDirection::BOTH,
                                 button1Handler);

  GPIO_A.setMode(PIN_BUT_2, GPIO::PinMode::INPUT);
  GPIO_A.setPullDirection(PIN_BUT_2, GPIO::PullDirection::PULL_UP);
  GPIO_A.enableExternalInterrupt(PIN_BUT_2, GPIO::TriggerDirection::BOTH,
                                 button2Handler);

  GPIO_A.setMode(PIN_BUT_3, GPIO::PinMode::INPUT);
  GPIO_A.setPullDirection(PIN_BUT_3, GPIO::PullDirection::PULL_UP);
  GPIO_A.enableExternalInterrupt(PIN_BUT_3, GPIO::TriggerDirection::BOTH,
                                 button3Handler);

  GPIO_A.setMode(PIN_BUT_4, GPIO::PinMode::INPUT);
  GPIO_A.setPullDirection(PIN_BUT_4, GPIO::PullDirection::PULL_UP);
  GPIO_A.enableExternalInterrupt(PIN_BUT_4, GPIO::TriggerDirection::BOTH,
                                 button4Handler);

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

    if (state != State::FIRING) {
      stop();
    }
  }
}