cmake_minimum_required(VERSION 3.13)

add_subdirectory(lib)

add_library(STM32Lib STATIC
    src/Startup.cpp
    src/Utils.cpp
    src/stm32f446/ADC.cpp
    src/stm32f446/Clock.cpp
    src/stm32f446/DAC.cpp
    src/stm32f446/DMA.cpp
    src/stm32f446/Flash.cpp
    src/stm32f446/GPIO.cpp
    src/stm32f446/Timer.cpp
    src/stm32f446/USART.cpp
    )

target_include_directories(STM32Lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stm32f446
    )

target_link_libraries(STM32Lib CMSIS)

target_compile_options(STM32Lib
    PRIVATE
        -ffreestanding
    )

target_compile_features(STM32Lib
    PUBLIC
        cxx_std_17
    )

add_subdirectory(app)
