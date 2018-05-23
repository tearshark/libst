LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lib_shark_task_static

LOCAL_MODULE_FILENAME := lib_shark_task

LOCAL_SRC_FILES := $(LOCAL_PATH)/libtask.cpp \
$(LOCAL_PATH)/../example/task_async.cpp \



LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include \
$(LOCAL_PATH)/../example

include $(BUILD_STATIC_LIBRARY)
