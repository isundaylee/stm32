################################################################################
# Environment setup
################################################################################

GCC_PREFIX  = arm-none-eabi
LLVM_PREFIX = /usr/local/opt/llvm/bin

CC 		= $(LLVM_PREFIX)/clang++
LD 		= $(GCC_PREFIX)-ld.bfd
OBJCOPY 	= $(GCC_PREFIX)-objcopy
DUMP 		= $(GCC_PREFIX)-objdump -d
GDB 		= $(GCC_PREFIX)-gdb
SIZE 		= $(GCC_PREFIX)-size
JLINK 		= /Applications/SEGGER/JLink/JLinkExe

################################################################################
# Directory setup
################################################################################

APP             = Sandbox
DEVICE          = stm32f446
DEVICE_INC_NAME = STM32F4xx
CPU_TYPE	= cortex-m4

DIR_BUILD       = build
DIR_SRC         = src
DIR_INC         = include
DIR_APP         = app
DIR_DEVICE_INC  = $(DIR_INC)/$(DEVICE)
DIR_DEVICE_SRC  = $(DIR_SRC)/$(DEVICE)

################################################################################
# Flags
################################################################################

CC_FLAGS 	= -target arm-none-eabi
# CC_FLAGS	+= -march=armv7-m
CC_FLAGS	+= -mthumb
CC_FLAGS	+= -mcpu=$(CPU_TYPE)
CC_FLAGS	+= -mfloat-abi=soft

CC_FLAGS	+= -g
CC_FLAGS	+= -O3
CC_FLAGS        += -std=c++17
CC_FLAGS 	+= -nostdlib
CC_FLAGS        += -ffreestanding
CC_FLAGS        += -fno-exceptions

CC_FLAGS 	+= -Wall
CC_FLAGS 	+= -Wextra
CC_FLAGS 	+= -Wold-style-cast
CC_FLAGS 	+= -Werror

CC_FLAGS 	+= -I./include
CC_FLAGS 	+= -I./include/$(DEVICE)
CC_FLAGS 	+= -isystem ./lib/CMSIS/Include
CC_FLAGS 	+= -isystem ./lib/CMSIS/Device/ST/$(DEVICE_INC_NAME)/Include

OBJS		= $(DIR_BUILD)/$(APP).o $(DIR_BUILD)/Startup.o $(DIR_BUILD)/Utils.o
DEVICE_OBJS	= $(patsubst $(DIR_SRC)/$(DEVICE)/%.cpp, $(DIR_BUILD)/%.o, $(wildcard $(DIR_DEVICE_SRC)/*.cpp))
HEADERS		= $(wildcard $(DIR_INC)/*.h)

################################################################################
# Compilation stage
################################################################################

.SUFFIXES:

$(DIR_BUILD)/$(APP).o: $(DIR_APP)/$(APP).cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

$(DIR_BUILD)/Startup.o: $(DIR_SRC)/Startup.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

$(DIR_BUILD)/Utils.o: $(DIR_SRC)/Utils.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

$(DIR_BUILD)/%.o: $(DIR_DEVICE_SRC)/%.cpp $(HEADERS)
	$(CC) -c $(CC_FLAGS) $< -o $@

################################################################################
# Linking stage
################################################################################

$(DIR_BUILD)/$(APP).elf: $(OBJS) $(DIR_SRC)/LinkerScript.lds $(DEVICE_OBJS)
	$(LD) -T $(DIR_SRC)/LinkerScript.lds $(OBJS) $(DEVICE_OBJS) $(LD_FLAGS) -o $@

$(DIR_BUILD)/$(APP).dump: $(DIR_BUILD)/$(APP).elf
	$(DUMP) $< >$@

$(DIR_BUILD)/$(APP).bin: $(DIR_BUILD)/$(APP).elf
	$(OBJCOPY) $< $@ -O binary

.DEFAULT_GOAL := $(DIR_BUILD)/$(APP).bin

################################################################################
# Utility functions
################################################################################

.PHONY: flash gdb clean size

flash: size $(DIR_BUILD)/$(APP).bin
	# st-flash --reset write $(DIR_BUILD)/$(APP).bin 0x08000000
	$(JLINK) -device STM32F446RE -if JTAG -speed 4000 -jtagconf -1,-1 -autoconnect 1 -CommanderScript $(DIR_SRC)/Flash.jlink

gdb: $(DIR_BUILD)/$(APP).elf
	$(GDB) $(DIR_BUILD)/$(APP).elf

clean:
	rm -f $(DIR_BUILD)/*

size: $(DIR_BUILD)/$(APP).elf
	$(SIZE) $(DIR_BUILD)/$(APP).elf
