LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := smajni
SMA_IMPL_SRCS := $(wildcard $(LOCAL_PATH)/../native/*.cpp)
JNI_IFACE_SRCS := $(wildcard $(LOCAL_PATH)/*.cpp)
LOCAL_SRC_FILES := $(SMA_IMPL_SRCS:$(LOCAL_PATH)/%=%) $(JNI_IFACE_SRCS:$(LOCAL_PATH)/%=%)
LOCAL_CFLAGS := -I../../../include/ -I../../../../include/
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
