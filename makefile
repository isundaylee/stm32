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

################################################################################
# Directory setup
################################################################################

APP             = Sandbox
DEVICE          = stm32l011

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
CC_FLAGS	+= -mcpu=cortex-m0
CC_FLAGS	+= -mthumb
CC_FLAGS	+= -mfloat-abi=soft

CC_FLAGS	+= -g
CC_FLAGS	+= -O2
CC_FLAGS        += -std=c++17
CC_FLAGS 	+= -nostdlib
CC_FLAGS        += -ffreestanding
CC_FLAGS        += -fno-exceptions

CC_FLAGS 	+= -Wall
CC_FLAGS 	+= -Wextra
CC_FLAGS 	+= -Wold-style-cast
CC_FLAGS 	+= -Werror

CC_FLAGS 	+= -I./include
CC_FLAGS 	+= -isystem ./lib/CMSIS/Include
CC_FLAGS 	+= -isystem ./lib/CMSIS/Device/ST/STM32L0xx/Include
CC_FLAGS	+= -DSTM32L011xx

CC_FLAGS_AS	= -target arm-none-eabi
CC_FLAGS_AS	+= -mcpu=cortex-m0
CC_FLAGS_AS	+= -mfloat-abi=soft

OBJS		= $(DIR_BUILD)/$(APP).o $(DIR_BUILD)/Boot.o $(DIR_BUILD)/Startup.o

################################################################################
# Compilation stage
################################################################################

$(DIR_BUILD)/$(APP).o: $(DIR_APP)/$(APP).cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

$(DIR_BUILD)/Startup.o: $(DIR_SRC)/Startup.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

$(DIR_BUILD)/Boot.o: $(DIR_SRC)/Boot.s
	$(CC) -c $(CC_FLAGS_AS) $< -o $@

################################################################################
# Linking stage
################################################################################

$(DIR_BUILD)/$(APP).elf: $(OBJS) $(DIR_SRC)/LinkerScript.lds
	$(LD) -T $(DIR_SRC)/LinkerScript.lds $(OBJS) $(LD_FLAGS) -o $@

$(DIR_BUILD)/$(APP).dump: $(DIR_BUILD)/$(APP).elf
	$(DUMP) $< >$@

$(DIR_BUILD)/$(APP).bin: $(DIR_BUILD)/$(APP).elf
	$(OBJCOPY) $< $@ -O binary

################################################################################
# Utility functions
################################################################################

flash: size $(DIR_BUILD)/$(APP).bin
	st-flash --reset write $(DIR_BUILD)/$(APP).bin 0x08000000

gdb: $(DIR_BUILD)/$(APP).elf
	$(GDB) $(DIR_BUILD)/$(APP).elf

clean:
	rm -f $(DIR_BUILD)/*

size: $(DIR_BUILD)/$(APP).elf
	$(SIZE) $(DIR_BUILD)/$(APP).elf
