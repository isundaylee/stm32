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
