
CUR_MK_DIR := $(call CUR_MK_DIR)
LOCAL_INCLUDES := include
LOCAL_DEFINES := 
LOCAL_CFLAGS :=
PLAT_OBJS := exceptions_el3.o exception_el3_handler.o










#OBJS += $(patsubst %,$(CUR_MK_DIR)%,$(PLAT_OBJS))
OBJS += $(addprefix  $(CUR_MK_DIR),$(PLAT_OBJS))
PLAT_CFLAGS +=$(addprefix -D,$(LOCAL_DEFINES)) $(addprefix -I$(CUR_MK_DIR),$(LOCAL_INCLUDES)) $(LOCAL_CFLAGS)
