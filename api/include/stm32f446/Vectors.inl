extern "C" void isrUSART1();
extern "C" void isrUSART2();
extern "C" void isrUSART3();
extern "C" void isrUSART6();

extern "C" void isrTimer2();
extern "C" void isrTimer3();
extern "C" void isrTimer4();
extern "C" void isrTimer5();

extern "C" void isrDMA1Stream0();
extern "C" void isrDMA1Stream1();
extern "C" void isrDMA1Stream2();
extern "C" void isrDMA1Stream3();
extern "C" void isrDMA1Stream4();
extern "C" void isrDMA1Stream5();
extern "C" void isrDMA1Stream6();
extern "C" void isrDMA1Stream7();
extern "C" void isrDMA2Stream0();
extern "C" void isrDMA2Stream1();
extern "C" void isrDMA2Stream2();
extern "C" void isrDMA2Stream3();
extern "C" void isrDMA2Stream4();
extern "C" void isrDMA2Stream5();
extern "C" void isrDMA2Stream6();
extern "C" void isrDMA2Stream7();

__attribute__ ((section (".isr_vectors")))
__attribute__ ((used))
static void (*_isrVectors[])() = {
    reinterpret_cast<void (*)()>(0x20000000 + 128 * 1024),
    &_reset,
    &_spin,
    &_hardFault,
    &_spin3,
    &_spin4,
    &_spin2,
    &_spin2,
    &_spin3,
    &_spin3,
    &_spin3,
    &_spin4,
    &_spin4,
    &_spin4,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &isrDMA1Stream0,
    &isrDMA1Stream1,
    &isrDMA1Stream2,
    &isrDMA1Stream3,
    &isrDMA1Stream4,
    &isrDMA1Stream5,
    &isrDMA1Stream6,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &_spin2,
    &isrTimer2,
    &isrTimer3,
    &isrTimer4,
    &_spin3,
    &_spin3,
    &_spin3,
    &_spin3,
    &_spin3,
    &_spin3,
    &isrUSART1,
    &isrUSART2,
    &isrUSART3,
    &_spin3,
    &_spin3,
    &_spin3,
    &_spin3,
    &_spin3,
    &_spin3,
    &_spin3,
    &isrDMA1Stream7,
    &_spin3,
    &_spin3,
    &isrTimer5,
    &_spin4,
    &_spin4,
    &_spin4,
    &_spin4,
    &_spin4,
    &isrDMA2Stream0,
    &isrDMA2Stream1,
    &isrDMA2Stream2,
    &isrDMA2Stream3,
    &isrDMA2Stream4,
    &_spin4,
    &_spin4,
    &_spin4,
    &_spin4,
    &_spin4,
    &_spin4,
    &_spin4,
    &isrDMA2Stream5,
    &isrDMA2Stream6,
    &isrDMA2Stream7,
    &isrUSART6,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin
};
