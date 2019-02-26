set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(TOOLCHAIN_PREFIX arm-none-eabi-)

execute_process(
    COMMAND which arm-none-eabi-ld
    OUTPUT_VARIABLE BINUTILS_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

get_filename_component(ARM_TOOLCHAIN_DIR ${BINUTILS_PATH} REALPATH)
set(ARM_TOOLCHAIN_DIR ${ARM_TOOLCHAIN_DIR}/../..)
get_filename_component(ARM_TOOLCHAIN_DIR ${ARM_TOOLCHAIN_DIR} ABSOLUTE)

# Without that flag CMake is not able to pass test compilation check
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER /usr/local/opt/llvm/bin/clang)
set(CMAKE_CXX_COMPILER /usr/local/opt/llvm/bin/clang++)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_AR ${TOOLCHAIN_PREFIX}ar)
set(CMAKE_LINKER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}ranlib)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE_UTIL ${TOOLCHAIN_PREFIX}size)
set(CMAKE_GDB ${TOOLCHAIN_PREFIX}gdb)

set(CMAKE_FIND_ROOT_PATH ${BINUTILS_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_compile_options(
    -target arm-none-eabi

    -fshort-enums

    -mthumb
    -mcpu=cortex-m4
    -mfloat-abi=soft
    -mfpu=fpv4-sp-d16
    -fno-exceptions

    -O3
    -g

    -Wall
    -Wextra
    -Wold-style-cast
    -Wno-address-of-packed-member
    -Werror
    )

link_directories(
    ${ARM_TOOLCHAIN_DIR}/lib/gcc/arm-none-eabi/7.3.1/thumb/v7-m
    )

set(CMAKE_CXX_LINK_EXECUTABLE
    "<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
    )

include_directories(SYSTEM
    ${ARM_TOOLCHAIN_DIR}/arm-none-eabi/include/c++/7.3.1
    ${ARM_TOOLCHAIN_DIR}/arm-none-eabi/include/c++/7.3.1/arm-none-eabi
    ${ARM_TOOLCHAIN_DIR}/arm-none-eabi/include/c++/7.3.1/backward
    ${ARM_TOOLCHAIN_DIR}/lib/gcc/arm-none-eabi/7.3.1/include
    ${ARM_TOOLCHAIN_DIR}/lib/gcc/arm-none-eabi/7.3.1/include-fixed
    ${ARM_TOOLCHAIN_DIR}/arm-none-eabi/include
    )
