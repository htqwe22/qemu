#CROSS_COMPILE ?= $(HOME)/.bin/aarch64-none-linux-10.3/bin/aarch64-none-linux-gnu-
CROSS_COMPILE ?= $(HOME)/.bin/aarch64-none-elf-10.3/bin/aarch64-none-elf-


Q ?=@

#MAKEFLAGS += --silent
ifeq ($(Q),@)
MAKEFLAGS += --no-print-directory
endif

AR  := $(CROSS_COMPILE)ar
CC  := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AS := $(CROSS_COMPILE)as
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

CFLAGS := -g -D__aarch64__ -O1 -fPIC #-march=armv8.5-a
CFLAGS += -Ikv_libc -Icommon -Iarch -Iarch/aarch64 -I driver
DEPFLAGS := -MD -MP

#CFLAGS := -fno-builtin -Wall -Wstrict-prototypes -fno-stack-protector -fno-common -nostdinc -static -fPIC
#LDFLAGS :=  -Bstatic -T test.lds -v
# or 链接时使用 $(LD) -Ttext=0xc4000000 -nostdlib test.o -o test
LDFLAGS := -Tlink.ld
#-nostdlib 

#CFLGAS += -ffunction-sections -fdata-section
#LDFLGAS += -Wl,--gc-sections
#C_FLAGS := $(shell find . -path "*.s4project" -prune -o -name "*.c" -print)

S_OBJS := start.o arch/aarch64/aarch64_misc.o
C_OBJS :=  main.o   
CXX_OBJS := 


OBJ_KV_LIBC := kv_libc/simple_vsprintf.o kv_libc/kv_string.o 
OBJ_COMMON :=  common/log.o arch/aarch64/aarch64_common.o
OBJ_DRV		:= #driver/uart/drv_uart.o
OBJ_DRV += driver/drv_pl011.o


C_OBJS += $(OBJ_KV_LIBC)
C_OBJS += $(OBJ_COMMON)
C_OBJS += $(OBJ_DRV)

V ?= n
BIN_NAME := main.bin
#########################################################################
# THE ORDER Bellow is very important. the first is the entrance
OBJS := $(S_OBJS) $(C_OBJS) $(CXX_OBJS) 
DEPS := $(OBJS:%.o=%.d)
 

all: $(BIN_NAME) 
.PHONY: all clean

SRC := $(C_OBJS:%.o=%.c)
SRS := $(S_OBJS:%.o=%.S)
SRCXX := $(CXX_OBJS:%.o=%.cpp)
ELFS := $(BIN_NAME:%.bin=%.elf)
MAP_NAME := $(BIN_NAME:%.bin=%.map)
DUMP_NAME := $(BIN_NAME:%.bin=%.asm)
LDFLAGS += -Map $(MAP_NAME) 

$(BIN_NAME):$(OBJS) link.ld
	$(Q)$(LD) ${LDFLAGS} $(OBJS) -o $(ELFS)
#	$(Q)$(NM) -n $(ELFS) > $(MAP_NAME)
	$(Q)$(OBJCOPY) -S -O binary $(ELFS) $@
#	$(Q)$(OBJDUMP) -S $(ELFS) > $(DUMP_NAME)
	$(Q)$(OBJDUMP) -D -S -d $(ELFS) > $(DUMP_NAME)
	
$(S_OBJS):%.o:%.S
	$(Q)$(CC) ${CFLAGS} ${DEPFLAGS} -c $< -o $@
$(C_OBJS):%.o:%.c
	$(Q)$(CC) ${CFLAGS} ${DEPFLAGS} -c $< -o $@
$(CXX_OBJS):%.o:%.cpp
	$(Q)$(CXX) ${CFLAGS} ${DEPFLAGS} -c $< -o $@

clean:
	$(Q)@rm -f $(BIN_NAME) $(OBJS) $(ELFS) $(MAP_NAME) $(DUMP_NAME) $(DEPS)

run:
	make
	$(Q)qemu-system-aarch64 -machine virt,gic-version=4,secure=on,virtualization=on -cpu neoverse-n2 -nographic -smp 1 -m 2G -kernel main.elf

debug:
	make
	$(Q)qemu-system-aarch64 -machine virt,gic-version=3,secure=on,virtualization=on -cpu neoverse-n2 -nographic -smp 1 -m 2G -kernel main.elf -S -s

dumpdtb:
	$(Q)qemu-system-aarch64 -machine virt,gic-version=3,secure=on,virtualization=on,dumpdtb=board.dtb -cpu neoverse-n2 -nographic -smp 4 -m 2G

dumpdts:
	$(Q)dtc -I dtb -O dts board.dtb -o board.dts
-include $(DEPS)

