# Copyright 2006 The Android Open Source Project

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= fastboot/protocol.c fastboot/engine.c fastboot/fastboot.c fastboot/usb_linux.c fastboot/util_linux.c
LOCAL_SRC_FILES+= tinystr.cpp tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp
LOCAL_SRC_FILES+= at_tok.cpp atchannel.cpp ril-daemon.cpp download.cpp file.cpp os_linux.cpp serialif.cpp quectel_log.cpp quectel_common.cpp quectel_crc.cpp
LOCAL_CFLAGS += -DUSE_FASTBOOT
LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE
LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE:= QFlash
LOCAL_LDLIBS:= -lm -llog
include $(BUILD_EXECUTABLE)
