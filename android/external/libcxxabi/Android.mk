#
# Copyright (C) 2014 The Android Open Source Project
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
#

LOCAL_PATH := $(call my-dir)

LIBCXXABI_SRC_FILES := \
    src/abort_message.cpp \
    src/cxa_aux_runtime.cpp \
    src/cxa_default_handlers.cpp \
    src/cxa_demangle.cpp \
    src/cxa_exception.cpp \
    src/cxa_exception_storage.cpp \
    src/cxa_guard.cpp \
    src/cxa_handlers.cpp \
    src/cxa_new_delete.cpp \
    src/cxa_personality.cpp \
    src/cxa_thread_atexit.cpp \
    src/cxa_unexpected.cpp \
    src/cxa_vector.cpp \
    src/cxa_virtual.cpp \
    src/exception.cpp \
    src/fallback_malloc.ipp \
    src/private_typeinfo.cpp \
    src/stdexcept.cpp \
    src/typeinfo.cpp \

LLVM_UNWIND_SRC_FILES := \
    src/Unwind/libunwind.cpp \
    src/Unwind/Unwind-EHABI.cpp \
    src/Unwind/Unwind-sjlj.c \
    src/Unwind/UnwindLevel1-gcc-ext.c \
    src/Unwind/UnwindLevel1.c \
    src/Unwind/UnwindRegistersSave.S \
    src/Unwind/UnwindRegistersRestore.S \

LIBCXXABI_INCLUDES := \
    $(LOCAL_PATH)/include/ \
    external/libcxx/include/ \

LIBCXXABI_RTTI_FLAG := -frtti
LIBCXXABI_CPPFLAGS := \
    -std=c++11 \
    -fexceptions \
    -Wall \
    -Wextra \
    -Wno-unused-function \
    -Werror \

include $(CLEAR_VARS)
LOCAL_MODULE := libunwind_llvm
LOCAL_CLANG := true
LOCAL_SRC_FILES_arm := $(LLVM_UNWIND_SRC_FILES)
LOCAL_C_INCLUDES := $(LIBCXXABI_INCLUDES)
LOCAL_CPPFLAGS := $(LIBCXXABI_CPPFLAGS)
LOCAL_CXX_STL := none
# src/Unwind/UnwindRegistersSave.S does not compile.
LOCAL_CLANG_ASFLAGS_arm += -no-integrated-as
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libc++abi
LOCAL_CLANG := true
LOCAL_SRC_FILES := $(LIBCXXABI_SRC_FILES)
LOCAL_C_INCLUDES := $(LIBCXXABI_INCLUDES)
LOCAL_CPPFLAGS := $(LIBCXXABI_CPPFLAGS) -DHAVE___CXA_THREAD_ATEXIT_IMPL
LOCAL_CPPFLAGS_arm := -DLIBCXXABI_USE_LLVM_UNWINDER=1
LOCAL_CPPFLAGS_arm64 := -DLIBCXXABI_USE_LLVM_UNWINDER=0
LOCAL_CPPFLAGS_mips := -DLIBCXXABI_USE_LLVM_UNWINDER=0
LOCAL_CPPFLAGS_mips64 := -DLIBCXXABI_USE_LLVM_UNWINDER=0
LOCAL_CPPFLAGS_x86 := -DLIBCXXABI_USE_LLVM_UNWINDER=0
LOCAL_CPPFLAGS_x86_64 := -DLIBCXXABI_USE_LLVM_UNWINDER=0
LOCAL_CXX_STL := none
LOCAL_RTTI_FLAG := $(LIBCXXABI_RTTI_FLAG)
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
# src/Unwind/UnwindRegistersSave.S does not compile.
LOCAL_CLANG_ASFLAGS_arm += -no-integrated-as
# When src/cxa_exception.cpp is compiled with Clang assembler
# __cxa_end_cleanup_impl, although marked as used, was discarded
# since it is used only in embedded assembly code.
# This caused the following warning when linking libc++.so:
# libc++_static.a(cxa_exception.o)(.text.__cxa_end_cleanup+0x2):
# warning: relocation refers to discarded section
# See also http://llvm.org/bugs/show_bug.cgi?id=21292.
LOCAL_CLANG_CFLAGS_arm += -no-integrated-as
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libc++abi
LOCAL_CLANG := true
LOCAL_SRC_FILES := $(LIBCXXABI_SRC_FILES)
LOCAL_C_INCLUDES := $(LIBCXXABI_INCLUDES)
LOCAL_CPPFLAGS := $(LIBCXXABI_CPPFLAGS)
LOCAL_CXX_STL := none

ifeq ($(HOST_OS),darwin)
LOCAL_SRC_FILES += $(LLVM_UNWIND_SRC_FILES) src/Unwind/Unwind_AppleExtras.cpp
# libcxxabi really doesn't like the non-LLVM assembler on Darwin
LOCAL_ASFLAGS += -integrated-as
LOCAL_CFLAGS += -integrated-as
LOCAL_CPPFLAGS += -integrated-as
endif

LOCAL_RTTI_FLAG := $(LIBCXXABI_RTTI_FLAG)
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_MULTILIB := both
include $(BUILD_HOST_STATIC_LIBRARY)
