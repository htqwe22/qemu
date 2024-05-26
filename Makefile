#CROSS_COMPILE ?= $(HOME)/.bin/aarch64-none-linux-10.3/bin/aarch64-none-linux-gnu-
CROSS_COMPILE ?= $(HOME)/.bin/aarch64-none-elf-10.3/bin/aarch64-none-elf-

AR  := $(CROSS_COMPILE)ar
CC  := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AS := $(CROSS_COMPILE)as
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

CFLAGS := -g -D__aarch64__ -O1
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

S_OBJS := start.o 
C_OBJS :=  main.o   
CXX_OBJS := 


OBJ_KV_LIBC := kv_libc/simple_vsprintf.o kv_libc/kv_string.o 
OBJ_COMMON :=  common/log.o
OBJ_DRV		:= driver/uart/drv_uart.o

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
.PHONY: all clean $(BIN_NAME)

SRC := $(C_OBJS:%.o=%.c)
SRS := $(S_OBJS:%.o=%.S)
SRCXX := $(CXX_OBJS:%.o=%.cpp)
ELFS := $(BIN_NAME:%.bin=%.elf)
MAP_NAME := $(BIN_NAME:%.bin=%.map)
DUMP_NAME := $(BIN_NAME:%.bin=%.asm)
LDFLAGS += -Map $(MAP_NAME) 

$(BIN_NAME):$(OBJS) link.ld
	$(LD) ${LDFLAGS} $(OBJS) -o $(ELFS)
#	$(NM) -n $(ELFS) > $(MAP_NAME)
#	$(OBJCOPY) -S -O binary $(ELFS) $@
	$(OBJDUMP) -S $(ELFS) > $(DUMP_NAME)
	
$(S_OBJS):%.o:%.S
	$(CC) ${CFLAGS} ${DEPFLAGS} -c $< -o $@
$(C_OBJS):%.o:%.c
	$(CC) ${CFLAGS} ${DEPFLAGS} -c $< -o $@
$(CXX_OBJS):%.o:%.cpp
	$(CXX) ${CFLAGS} ${DEPFLAGS} -c $< -o $@

clean:
	@rm -f $(BIN_NAME) $(OBJS) $(ELFS) $(MAP_NAME) $(DUMP_NAME) $(DEPS)

run:
	qemu-system-aarch64 -machine raspi4b -nographic -m 2G -kernel main.elf

debug:	
	qemu-system-aarch64 -machine raspi4b -nographic -m 2G -kernel main.elf -S -s

-include $(DEPS)

