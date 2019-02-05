add_executable(Sandbox
    Sandbox.cpp
    )

target_link_libraries(Sandbox STM32Lib)

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
    )
