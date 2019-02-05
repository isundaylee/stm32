add_library(CMSIS INTERFACE)

target_include_directories(CMSIS
    SYSTEM
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/CMSIS/Device/ST/STM32F4xx/Include
    )
