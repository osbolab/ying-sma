LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := smajni
LOCAL_SRC_FILES := jninodecontainer.cpp
LOCAL_CFLAGS := -I../../../../include/ -I../../../../../include/
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
