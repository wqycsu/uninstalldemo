LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := uninstall
LOCAL_SRC_FILES := AppUninstallListen.cpp
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_LDLIBS += -llog
LOCAL_LDLIBS += -lz
include $(BUILD_SHARED_LIBRARY)