override CUR_MK_DIR := $(patsubst $(TOP_DIR)/%,%,$(dir $(lastword $(MAKEFILE_LIST))))

override LOCAL_INCLUDES := .
override LOCAL_DEFINES := 
override LOCAL_CFLAGS :=
override LOCAL_OBJS := drv_pl011.o gicv3_basic.o gicv3_lpis.o gicv3_cpuif.o drv_sys_timer.o






#OBJS += $(patsubst %,$(CUR_MK_DIR)%,$(LOCAL_OBJS))
OBJS += $(addprefix  $(CUR_MK_DIR),$(LOCAL_OBJS))
CFLAGS += $(addprefix -D,$(LOCAL_DEFINES)) $(addprefix -I$(CUR_MK_DIR),$(LOCAL_INCLUDES)) $(LOCAL_CFLAGS)
