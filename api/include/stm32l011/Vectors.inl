extern "C" void isrRTC();
extern "C" void isrEXTI01();
extern "C" void isrEXTI23();
extern "C" void isrEXTI415();
extern "C" void isrSysTick();
extern "C" void isrTIM2();

__attribute__ ((section (".isr_vectors")))
__attribute__ ((used))
static void (*_isrVectors[])() = {
    reinterpret_cast<void (*)()>(0x20000000 + 2 * 1024),
    &_reset,
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
    &isrSysTick,
    &_spin,
    &_spin,
    &isrRTC,
    &_spin,
    &_spin,
    &isrEXTI01,
    &isrEXTI23,
    &isrEXTI415,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &_spin,
    &isrTIM2,
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
