include device/softwinner/common/BoardConfigCommon.mk

TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP := true
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := cortex-a7

TARGET_NO_BOOTLOADER := true

TARGET_BOARD_PLATFORM := octopus
TARGET_BOOTLOADER_BOARD_NAME := exdroid
TARGET_BOOTLOADER_NAME := exdroid

BOARD_EGL_CFG := device/softwinner/octopus-common/egl/egl.cfg
BOARD_KERNEL_BASE := 0x40000000

BOARD_CHARGER_ENABLE_SUSPEND := true

BOARD_SEPOLICY_DIRS := \
    device/softwinner/octopus-common/sepolicy

#Justin Porting for BPI-M3 Root Start
BOARD_SEPOLICY_UNION := \
	bluetooth.te \
	device.te \
	file_contexts \
	genfs_contexts \
	init.te \
	kernel.te \
	mediaserver.te \
	netd.te \
	preinstall.te \
	recovery.te \
        rild.te \
	sayeye.te \
	sensors.te \
	service_contexts \
	shell.te \
	surfaceflinger.te \
	system_app.te \
	system_server.te \
	unconfined.te \
	vold.te \
	wpa.te \
    file.te \
    logger.te \
	platform_app.te
#Justin Porting for BPI-M3  Root End
USE_OPENGL_RENDERER := true
