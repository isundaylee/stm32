#include <DeviceHeader.h>

const uint32_t GPIO_MODE_INPUT = 0b00;
const uint32_t GPIO_MODE_OUTPUT = 0b01;
const uint32_t GPIO_MODE_ALTERNATE = 0b10;
const uint32_t GPIO_MODE_ANALOG = 0b11;

class GPIO {
private:
  bool initialized_ = false;
  GPIO_TypeDef *gpio_;

public:
  GPIO(GPIO_TypeDef *gpio) { gpio_ = gpio; }

  void initialize();

  void setMode(int pin, uint32_t mode, uint32_t alternate = 0);

  void set(int pin);
  void clear(int pin);
  void toggle(int pin);
  bool get(int pin);
};

extern GPIO GPIO_A;
extern GPIO GPIO_B;
extern GPIO GPIO_C;
