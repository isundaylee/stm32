add_executable(stm32f446_examples_gpio_blinky
    main.cpp
    )

target_link_libraries(stm32f446_examples_gpio_blinky STM32F446Lib)

target_compile_options(stm32f446_examples_gpio_blinky
    PRIVATE
        -nostdlib
        -ffreestanding
    )

target_link_options(stm32f446_examples_gpio_blinky
    PRIVATE
        -T ${CMAKE_CURRENT_SOURCE_DIR}/../../../common/LinkerScript.lds
        -nostdlib
        -ffreestanding
        -Wl,--whole-archive
    )

add_custom_command(
    OUTPUT stm32f446_examples_gpio_blinky.bin
    COMMAND ${CMAKE_OBJCOPY} --output-format=binary $<TARGET_FILE:stm32f446_examples_gpio_blinky> stm32f446_examples_gpio_blinky.bin
    DEPENDS stm32f446_examples_gpio_blinky
    COMMENT "Generating stm32f446_examples_gpio_blinky.bin"
    )

add_custom_target(flash_stm32f446_examples_gpio_blinky
    COMMAND JLinkExe -device STM32F446RE -if JTAG -speed 4000 -jtagconf -1,-1 -autoconnect 1 -CommanderScript ${CMAKE_CURRENT_SOURCE_DIR}/../../../Flash.jlink
    DEPENDS stm32f446_examples_gpio_blinky.bin)
