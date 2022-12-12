TARGET_PATH:= $(call my-dir)
include $(ENV_CLEAR)

include $(TARGET_PATH)/../../config/mpp_config.mk

ifeq ($(MPPCFG_COMPILE_DYNAMIC_LIB), Y)

TARGET_SRC := \
	video_render.c \
	video_render_linux.cpp \
	CedarXNativeRenderer.cpp \

#	audio_render.c \
#	audio_render_stagefright.c \
#	subtitle_render_android.cpp

TARGET_INC := \
    $(TARGET_TOP)/system/public/include/vo \
    $(TARGET_TOP)/middleware/config \
    $(TARGET_TOP)/middleware/include \
    $(TARGET_TOP)/middleware/include/utils \
    $(TARGET_TOP)/middleware/include/media \
    $(TARGET_TOP)/middleware/media/include \
    $(TARGET_TOP)/middleware/media/include/component \
    $(TARGET_TOP)/middleware/media/include/utils \
    $(TARGET_TOP)/middleware/media/include/include_render \
    $(TARGET_TOP)/middleware/media/LIBRARY/libcedarc/include \
    ${TARGET_PATH} \

TARGET_CFLAGS += -fPIC -Wall -Wno-multichar
TARGET_CPPFLAGS += -fPIC -Wall -Wno-multichar

TARGET_CPPFLAGS += $(CEDARX_EXT_CFLAGS)
TARGET_CFLAGS += $(CEDARX_EXT_CFLAGS)

TARGET_MODULE:= libcedarxrender

include $(BUILD_SHARED_LIB)

endif


ifeq ($(MPPCFG_COMPILE_STATIC_LIB), Y)

include $(ENV_CLEAR)

TARGET_SRC := \
	video_render.c \
	video_render_linux.cpp \
	CedarXNativeRenderer.cpp \

#	audio_render.c \
#	audio_render_stagefright.c \
#	subtitle_render_android.cpp

TARGET_INC := \
    $(TARGET_TOP)/system/public/include/vo \
    $(TARGET_TOP)/middleware/config \
    $(TARGET_TOP)/middleware/include \
    $(TARGET_TOP)/middleware/include/utils \
    $(TARGET_TOP)/middleware/include/media \
    $(TARGET_TOP)/middleware/media/include \
    $(TARGET_TOP)/middleware/media/include/component \
    $(TARGET_TOP)/middleware/media/include/utils \
    $(TARGET_TOP)/middleware/media/include/include_render \
    $(TARGET_TOP)/middleware/media/LIBRARY/libcedarc/include \
    ${TARGET_PATH} \

TARGET_CFLAGS += -fPIC -Wall -Wno-multichar
TARGET_CPPFLAGS += -fPIC -Wall -Wno-multichar

TARGET_CPPFLAGS += $(CEDARX_EXT_CFLAGS)
TARGET_CFLAGS += $(CEDARX_EXT_CFLAGS)

TARGET_MODULE:= libcedarxrender

include $(BUILD_STATIC_LIB)

endif

