# Copyright (c) 2011-2014, Intel Corporation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation and/or
# other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

LOCAL_PATH := $(call my-dir)

####################
# Common definitions

common_src_files := \
        Socket.cpp \
        ListeningSocket.cpp \
        ConnectionSocket.cpp \
        Message.cpp \
        RequestMessage.cpp \
        AnswerMessage.cpp \
        RemoteProcessorServer.cpp \
        RemoteProcessorServerBuilder.cpp

common_module := libremote-processor
common_module_tags := optional

common_cflags := \
        -Wall \
        -Werror \
        -Wextra \
        -Wno-unused-parameter \

#############################
# Target build

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(common_src_files)

LOCAL_STATIC_LIBRARIES := libpfw_utility

LOCAL_CFLAGS := $(common_cflags)

LOCAL_MODULE := $(common_module)
LOCAL_MODULE_OWNER := intel
LOCAL_MODULE_TAGS := $(common_module_tags)

include $(BUILD_SHARED_LIBRARY)

##############################
# Host build

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(common_src_files)

LOCAL_STATIC_LIBRARIES := libpfw_utility_host

LOCAL_CFLAGS := $(common_cflags) -pthread
LOCAL_LDLIBS := -lpthread

LOCAL_MODULE := $(common_module)_host
LOCAL_MODULE_OWNER := intel
LOCAL_MODULE_TAGS := $(common_module_tags)


include $(BUILD_HOST_SHARED_LIBRARY)
