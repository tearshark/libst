LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lib_shark_task_static
LOCAL_MODULE_FILENAME := lib_shark_task
LOCAL_SRC_FILES := libs/Android/$(TARGET_ARCH_ABI)/libvfxbase.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include

include $(PREBUILT_STATIC_LIBRARY)
