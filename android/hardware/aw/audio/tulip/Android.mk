# Copyright (C) 2011 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

# libAwHeadpSurround
include $(CLEAR_VARS)
LOCAL_MODULE := libAwHeadpSurround
LOCAL_SRC_FILES := audio_3d_surround/libAwHeadpSurround.so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .so
LOCAL_MULTILIB := 32
include $(BUILD_PREBUILT)


include $(CLEAR_VARS)
LOCAL_MODULE := audio.primary.$(TARGET_BOARD_PLATFORM)

#LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_SRC_FILES := audio_hw.c audio_iface.c sunxi_volume.c audio_3d_surround/audio_3d_surround.c

#ifneq ($(SW_BOARD_HAVE_3G), true)
#LOCAL_SRC_FILES += audio_ril_stub.c 
#else
#LOCAL_SHARED_LIBRARIES := libaudio_ril
#endif

LOCAL_C_INCLUDES += \
	external/tinyalsa/include \
	system/media/audio_utils/include \
	system/media/audio_effects/include \
	system/media/audio_route/include
	

LOCAL_SHARED_LIBRARIES += liblog libcutils libtinyalsa libaudioutils libdl libcodec_audio libril_audio libaudioroute
LOCAL_SHARED_LIBRARIES_32 += libAwHeadpSurround
LOCAL_MODULE_TAGS := optional
include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under, $(LOCAL_PATH))



