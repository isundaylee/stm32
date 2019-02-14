set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(TOOLCHAIN_PREFIX arm-none-eabi-)

execute_process(
    COMMAND which ${TOOLCHAIN_PREFIX}gcc
    OUTPUT_VARIABLE BINUTILS_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
get_filename_component(ARM_TOOLCHAIN_DIR ${BINUTILS_PATH} REALPATH)
set(ARM_TOOLCHAIN_DIR ${ARM_TOOLCHAIN_DIR}/../..)
get_filename_component(ARM_TOOLCHAIN_DIR ${ARM_TOOLCHAIN_DIR} ABSOLUTE)

# Without that flag CMake is not able to pass test compilation check
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_AR ${TOOLCHAIN_PREFIX}ar)
set(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}ranlib)
set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE_UTIL ${TOOLCHAIN_PREFIX}size)
set(CMAKE_GDB ${TOOLCHAIN_PREFIX}gdb)

set(CMAKE_FIND_ROOT_PATH ${BINUTILS_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_compile_options(
    -mthumb
    -mcpu=cortex-m4
    -mfloat-abi=hard
    -fno-exceptions

    -O3
    -g

    -Wall
    -Wextra
    -Wold-style-cast
    -Werror
    )
  
include_directories(SYSTEM
    ${ARM_TOOLCHAIN_DIR}/arm-none-eabi/include/c++/7.3.1
    ${ARM_TOOLCHAIN_DIR}/arm-none-eabi/include/c++/7.3.1/arm-none-eabi
    ${ARM_TOOLCHAIN_DIR}/arm-none-eabi/include/c++/7.3.1/backward
    ${ARM_TOOLCHAIN_DIR}/lib/gcc/arm-none-eabi/7.3.1/include
    ${ARM_TOOLCHAIN_DIR}/lib/gcc/arm-none-eabi/7.3.1/include-fixed
    ${ARM_TOOLCHAIN_DIR}/arm-none-eabi/include
    )
