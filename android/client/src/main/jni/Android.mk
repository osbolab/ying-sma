LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := smajni
LOCAL_SRC_FILES := hellojniactivity.c

include $(BUILD_SHARED_LIBRARY)
