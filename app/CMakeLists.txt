add_executable(Sandbox
    Sandbox.cpp
    )

target_link_libraries(Sandbox STM32F446Lib)

target_compile_options(Sandbox
    PRIVATE
        -nostdlib
        -ffreestanding
    )

target_link_options(Sandbox
    PRIVATE
        -T ${CMAKE_CURRENT_SOURCE_DIR}/LinkerScript.lds
        -nostdlib
        -ffreestanding
        -Wl,--whole-archive
    )

add_custom_command(
    OUTPUT Sandbox.bin
    COMMAND ${CMAKE_OBJCOPY} --output-format=binary $<TARGET_FILE:Sandbox> Sandbox.bin
    DEPENDS Sandbox
    COMMENT "Generating Sandbox.bin"
    )

add_custom_target(flash_Sandbox
    COMMAND JLinkExe -device STM32F446RE -if JTAG -speed 4000 -jtagconf -1,-1 -autoconnect 1 -CommanderScript ${CMAKE_CURRENT_SOURCE_DIR}/Flash.jlink
    DEPENDS Sandbox.bin)
