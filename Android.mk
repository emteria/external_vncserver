LOCAL_PATH := $(call my-dir)

##############################################

include $(CLEAR_VARS)

LOCAL_MODULE := vncd
LOCAL_INIT_RC := vncd.rc

LOCAL_AIDL_INCLUDES := com/rotationhelper/IRotationHelper.aidl

LOCAL_SRC_FILES := \
    input/suinput.cpp \
    input/input.cpp \
    input/clipboard.cpp \
    vncd.cpp \
    com/rotationhelper/IRotationHelper.aidl \

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/common \
    $(LOCAL_PATH)/input \
    $(LOCAL_PATH)/screen \
    external/zlib \
    external/libvncserver \
    $(LOCAL_PATH)/com/rotationhelper 

LOCAL_CFLAGS += \
    -Wall \
    -O3 \
    -Wno-unused-parameter \
    -DLIBVNCSERVER_WITH_WEBSOCKETS \
    -DLIBVNCSERVER_HAVE_LIBPNG \
    -DLIBVNCSERVER_HAVE_ZLIB \
    -DLIBVNCSERVER_HAVE_LIBJPEG

LOCAL_LDLIBS += \
    -llog \
    -lz \
    -ldl

LOCAL_SHARED_LIBRARIES := \
    libcrypto \
    libjpeg \
    liblog \
    libpng \
    libssl \
    libgui \
    libui \
    libbinder \
    libcutils \
    libutils

LOCAL_STATIC_LIBRARIES := \
    libvncserver \

include $(BUILD_EXECUTABLE)
