LOCAL_PATH := $(call my-dir)

SMA_ROOT := $(abspath ../../../../..)
SMA_SRC := $(SMA_ROOT)/src
SMA_INC := $(SMA_ROOT)/include

include $(CLEAR_VARS)
LOCAL_MODULE := smanative

SMA_SOURCES := $(wildcard $(SMA_SRC)/*.cpp)
SMA_SOURCES += $(wildcard $(SMA_SRC)/**/*.cpp)
SMA_SOURCES += $(wildcard $(SMA_SRC)/**/**/*.cpp)

JNI_SOURCES := $(wildcard $(LOCAL_PATH)/*.cpp)

LOCAL_SRC_FILES := $(SMA_SOURCES:$(LOCAL_PATH)/%=%) $(JNI_SOURCES:$(LOCAL_PATH)/%=%)
LOCAL_CFLAGS := -I../../../include/ -I$(SMA_INC)
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
