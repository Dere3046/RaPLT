LOCAL_PATH := $(call my-dir)

# --- raplt core library (static) ---
include $(CLEAR_VARS)
LOCAL_MODULE := raplt_static
LOCAL_SRC_FILES := \
    $(LOCAL_PATH)/../../../src/raplt_core.c \
    $(LOCAL_PATH)/../../../src/raplt_elf.c \
    $(LOCAL_PATH)/../../../src/raplt_hash.c \
    $(LOCAL_PATH)/../../../src/raplt_signal.c \
    $(LOCAL_PATH)/../../../src/raplt_util.c \
    $(LOCAL_PATH)/../../../src/raplt_recon.c \
    $(LOCAL_PATH)/../../../src/raplt_dlopen.c \
    $(LOCAL_PATH)/../../../src/raplt_cfi.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../include
LOCAL_CFLAGS := -Wall -fvisibility=hidden -Wno-unused-parameter -Wno-unused-function
include $(BUILD_STATIC_LIBRARY)

# --- test executable ---
include $(CLEAR_VARS)
LOCAL_MODULE := test_raplt
LOCAL_SRC_FILES := test_raplt.c
LOCAL_STATIC_LIBRARIES := raplt_static
LOCAL_LDLIBS := -llog -ldl
include $(BUILD_EXECUTABLE)
