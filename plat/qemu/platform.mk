
override CUR_MK_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
override LOCAL_INCLUDES := include
override LOCAL_DEFINES := 
override LOCAL_CFLAGS :=
override LOCAL_OBJS := exceptions_el3.o










#OBJS += $(patsubst %,$(CUR_MK_DIR)%,$(LOCAL_OBJS))
OBJS += $(addprefix  $(CUR_MK_DIR),$(LOCAL_OBJS))
PLAT_CFLAGS +=$(addprefix -D,$(LOCAL_DEFINES)) $(addprefix -I$(CUR_MK_DIR),$(LOCAL_INCLUDES)) $(LOCAL_CFLAGS)
