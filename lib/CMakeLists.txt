add_library(CMSIS INTERFACE)

target_include_directories(CMSIS
    SYSTEM
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Device/ST/STM32F4xx/Include
    )
    
add_library(printf
    printf/printf.c
    )

target_include_directories(printf
    SYSTEM
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/printf
    )

target_compile_definitions(printf
    PRIVATE
        PRINTF_DISABLE_SUPPORT_LONG_LONG
    )
