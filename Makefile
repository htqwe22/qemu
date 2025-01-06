PLAT ?= qemu  #rk3568

VERSION_MAJOR			:= 0
VERSION_MINOR			:= 1
VERSION_PATCH			:= 0
VERSION				:= ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}

export TOP_DIR=$(shell pwd)
export MAKE_TOOLS_DIR := $(TOP_DIR)/make_tools
ARCH_DIR := $(TOP_DIR)/arch/aarch64
PLAT_DIR = plat/$(strip $(PLAT))
TEST_DIR := $(TOP_DIR)/test

CROSS_COMPILE ?= $(HOME)/.bin/aarch64-none-elf-10.3/bin/aarch64-none-elf-
PLAT:=$(strip $(PLAT))

include ${MAKE_TOOLS_DIR}/build_macros.mk
include $(MAKE_TOOLS_DIR)/build_flags.mk

V ?= 0
ifeq ($(V),1)
Q :=
ECHO := echo
else
Q :=@
ECHO :=
MAKEFLAGS += --silent --no-print-directory
endif

AR  := $(CROSS_COMPILE)ar
CC  := $(CROSS_COMPILE)gcc
CXX := $(CROSS_COMPILE)g++
AS := $(CROSS_COMPILE)as
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

OBJS :=

CFLAGS += -DBUILD_MESSAGE_TIMESTAMP='__TIME__", "__DATE__'
CFLAGS += -g -D__aarch64__ -fno-pic #-O1 -fPIC #-march=armv8.5-a

#CFLAGS += -Wall -Wno-unused-function -Wno-unused-variable -Wunreachable-code -Wno-format-truncation -Wint-to-pointer-cast
CFLAGS += -Ikv_libc -Icommon -Iarch -Iarch/aarch64 -I driver
DEPFLAGS := -MD -MP

#CFLAGS := -fno-builtin -Wall -Wstrict-prototypes -fno-stack-protector -fno-common -nostdinc -static -fPIC
#LDFLAGS :=  -Bstatic -T test.lds -v
# or 链接时使用 $(LD) -Ttext=0xc4000000 -nostdlib test.o -o test
LINK_FILE := $(PLAT_DIR)/link.ld
LDFLAGS := -T$(LINK_FILE) -nostdlib 
LDFLAGS += --no-dynamic-linker -pie
CFLGAS += -ffunction-sections -fdata-section
#LDFLGAS += -Wl,--gc-sections -pie
#C_FLAGS := $(shell find . -path "*.s4project" -prune -o -name "*.c" -print)
include $(PLAT_DIR)/platform.mk
include $(ARCH_DIR)/firmware.mk
include $(TEST_DIR)/firmware.mk

S_OBJS := start.o 
C_OBJS :=  main.o   
CXX_OBJS := 


OBJ_KV_LIBC := kv_libc/simple_vsprintf.o kv_libc/kv_string.o 
OBJ_COMMON :=  common/log.o common/shell.o
OBJ_DRV		:= #driver/uart/drv_uart.o
OBJ_DRV += driver/drv_pl011.o


C_OBJS += $(OBJ_KV_LIBC)
C_OBJS += $(OBJ_COMMON)
C_OBJS += $(OBJ_DRV)


BIN_NAME := main.bin
M ?= virt
#########################################################################
# THE ORDER Bellow is very important. the first is the entrance

CFLAGS += $(PLAT_CFLAGS)
OBJS += $(S_OBJS) $(C_OBJS) $(CXX_OBJS)
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

$(BIN_NAME):$(OBJS) $(LINK_FILE)
	$(Q)$(LD) ${LDFLAGS} $(OBJS) -o $(ELFS)
#	$(Q)$(NM) -n $(ELFS) > $(MAP_NAME)
	$(Q)$(OBJCOPY) -S -O binary $(ELFS) $@
#	$(Q)$(OBJDUMP) -S $(ELFS) > $(DUMP_NAME)
	$(Q)$(OBJDUMP) -D -S -d $(ELFS) > $(DUMP_NAME)
	
#$(S_OBJS):%.o:%.S
#	$(Q)$(CC) ${CFLAGS} ${DEPFLAGS} -c $< -o $@
#$(C_OBJS):%.o:%.c
#	$(Q)$(CC) ${CFLAGS} ${DEPFLAGS} -c $< -o $@
#$(CXX_OBJS):%.o:%.cpp
#	$(Q)$(CXX) ${CFLAGS} ${DEPFLAGS} -c $< -o $@



%.o:%.S
	$(Q)$(CC) ${CFLAGS} ${DEPFLAGS} -c $< -o $@
%.o:%.c
	$(Q)$(CC) ${CFLAGS} ${DEPFLAGS} -c $< -o $@
%.o:%.cpp
	$(Q)$(CXX) ${CFLAGS} ${DEPFLAGS} -c $< -o $@

clean:
	$(Q)@rm -f $(BIN_NAME) $(OBJS) $(ELFS) $(MAP_NAME) $(DUMP_NAME) $(DEPS)

run:
	$(Q)echo "please use ./b.sh run instead"
#	make
#	$(Q)qemu-system-aarch64 -machine $(M),gic-version=3,secure=on,virtualization=on -cpu neoverse-n2 -nographic -smp 1 -m 2G -kernel main.elf

debug:
	$(Q)echo "please use ./b.sh debug instead"
#	make
#	$(Q)qemu-system-aarch64 -machine  $(M),gic-version=3,secure=on,virtualization=on -cpu neoverse-n2 -nographic -smp 1 -m 2G -kernel main.elf -S -s

dumpdtb:
	$(Q)qemu-system-aarch64 -machine  $(M),gic-version=3,secure=on,virtualization=on,dumpdtb=board.dtb -cpu neoverse-n2 -nographic -smp 4 -m 2G

dumpdts:
	$(Q)dtc -I dtb -O dts board.dtb -o board.dts
-include $(DEPS)

# 以下是将链接脚本汇编文件转换为链接脚本的方法
#$(LINKERFILE): $(LINKERFILE_SRC)
#	$(Q)$(CC) $(COMMON_FLAGS) $(INCLUDES) -P -E -D__LINKER__ -MMD -MF $@.d -MT $@ -o $@ $<
#	$$(Q)$($(ARCH)-cpp) -E $$(CPPFLAGS) $(BL_CPPFLAGS) $(TF_CFLAGS_$(ARCH)) -P -x assembler-with-cpp -D__LINKER__ $(MAKE_DEP) -o $$@ $$<
#	or gcc -E -P -x c -Iinclude/ ld.S > ld.ld
#-E：告诉 GCC 只进行预处理操作
#-P：防止预处理器生成行标记（#line 指令），这样生成的文件会更加干净
#-x c：告诉编译器将输入文件视为 C 语言文件，即使它是一个汇编文件
#-include $(LINKERFILE).d

