add_library(STM32F446Lib STATIC
    src/Startup.cpp
    src/Utils.cpp
    src/stm32f446/ADC.cpp
    src/stm32f446/Clock.cpp
    src/stm32f446/CoroSPI.cpp
    src/stm32f446/DAC.cpp
    src/stm32f446/DMA.cpp
    src/stm32f446/Flash.cpp
    src/stm32f446/GPIO.cpp
    src/stm32f446/Tick.cpp
    src/stm32f446/Timer.cpp
    src/stm32f446/USART.cpp
    src/stm32f446/SPI.cpp
    src/enc28j60/ENC28J60.cpp
    src/enc28j60/Core.cpp
    src/enc28j60/Transceiver.cpp
    )

target_include_directories(STM32F446Lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stm32f446
    )

target_link_libraries(STM32F446Lib
    PUBLIC
        CMSIS
        printf
        coro
    PRIVATE
        gcc
    )

target_compile_options(STM32F446Lib
    PRIVATE
        -ffreestanding
    )

target_compile_features(STM32F446Lib
    PUBLIC
        cxx_std_17
    )

################################################################################

add_library(STM32L011Lib STATIC
    src/stm32l011/StartupL011.cpp
    src/stm32l011/UtilsL011.cpp
    src/stm32l011/Clock.cpp
    src/stm32l011/GPIO.cpp
    src/stm32l011/I2C.cpp
    src/stm32l011/RealTimeClock.cpp
    src/stm32l011/Tick.cpp
    src/stm32l011/Timer.cpp
    )

target_include_directories(STM32L011Lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/stm32l011
    )

target_link_libraries(STM32L011Lib
    PUBLIC
        CMSIS
    PRIVATE
        gcc
    )

target_compile_options(STM32L011Lib
    PRIVATE
        -ffreestanding
    )

target_compile_features(STM32L011Lib
    PUBLIC
        cxx_std_17
    )
