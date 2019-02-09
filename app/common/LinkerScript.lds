MEMORY
{
    sram(WAIL) : ORIGIN = 0x20000000, LENGTH = 128K
    flash(RX)  : ORIGIN = 0x08000000, LENGTH = 512K
}

SECTIONS
{
    .text :
    {
        * (.isr_vectors)
        * (.text)
        * (.rodata)
    } > flash

    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        KEEP(*(SORT(.bss.*)))
        KEEP(*(.bss))
        . = ALIGN(4);
        . += 4;
        __bss_end__ = .;
    } > sram

    .data :
    {
        . = ALIGN(4);
        __data_start_load__ = LOADADDR(.data);
        __data_start__ = .;
        KEEP(*(SORT(.data.*)))
        KEEP(*(.data))
        . = ALIGN(4);
        . += 4;
        __data_end__ = .;
    } > sram AT > flash

    .init_array :
    {
        __init_array_start__ = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end__ = .;
        . += 4;
    } > flash
}
