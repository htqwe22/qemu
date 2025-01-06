override CUR_MK_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
#$(warning "CUR_MK_DIR is $(CUR_MK_DIR)")

override LOCAL_INCLUDES := 
override LOCAL_DEFINES := 
override LOCAL_CFLAGS :=
override LOCAL_OBJS := aarch64_misc.o aarch64_common.o mem_map.o aarch64_mmu.o 

override LOCAL_OBJS += stack_frame.o





#OBJS += $(patsubst %,$(CUR_MK_DIR)%,$(LOCAL_OBJS))

OBJS += $(addprefix  $(CUR_MK_DIR),$(LOCAL_OBJS))

CFLAGS +=$(addprefix -D,$(LOCAL_DEFINES)) $(addprefix -I$(CUR_MK_DIR),$(LOCAL_INCLUDES)) $(LOCAL_CFLAGS)
