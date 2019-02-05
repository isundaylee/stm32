add_executable(Sandbox
    ../src/Startup.cpp
    Sandbox.cpp
    )

target_link_libraries(Sandbox STM32Lib gcc)

target_compile_options(Sandbox
    PRIVATE
        -nostdlib
        -ffreestanding
    )

target_link_options(Sandbox
    PRIVATE
        -T ${CMAKE_CURRENT_SOURCE_DIR}/../src/LinkerScript.lds
        -nostdlib
        -ffreestanding
    )
