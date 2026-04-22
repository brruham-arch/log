LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := keylogger
LOCAL_SRC_FILES := main.cpp
LOCAL_LDLIBS := -llog
LOCAL_STATIC_LIBRARIES := curl
LOCAL_CPPFLAGS := -std=c++17 -fvisibility=hidden
include $(BUILD_SHARED_LIBRARY)

# Import libcurl
$(call import-module, curl)