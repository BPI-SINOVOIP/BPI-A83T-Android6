$(call inherit-product, build/target/product/full_base.mk)
$(call inherit-product, device/softwinner/octopus-common/octopus-common.mk)
$(call inherit-product-if-exists, device/softwinner/bpi-m3-lcd5/modules/modules.mk)

DEVICE_PACKAGE_OVERLAYS := device/softwinner/bpi-m3-lcd5/overlay \
                           $(DEVICE_PACKAGE_OVERLAYS)

#PRODUCT_PACKAGES += gatord

PRODUCT_PACKAGES += \
    ESFileExplorer \
    VideoPlayer \
    Bluetooth \
    Launcher3
#   PartnerChromeCustomizationsProvider

PRODUCT_COPY_FILES += \
    device/softwinner/bpi-m3-lcd5/kernel:kernel \
    device/softwinner/bpi-m3-lcd5/fstab.sun8i:root/fstab.sun8i \
    device/softwinner/bpi-m3-lcd5/init.sun8i.rc:root/init.sun8i.rc \
    device/softwinner/bpi-m3-lcd5/init.recovery.sun8i.rc:root/init.recovery.sun8i.rc \
    device/softwinner/bpi-m3-lcd5/ueventd.sun8i.rc:root/ueventd.sun8i.rc \
    device/softwinner/common/verity/rsa_key/verity_key:root/verity_key \
    device/softwinner/bpi-m3-lcd5/modules/modules/nand.ko:root/nand.ko \
    device/softwinner/bpi-m3-lcd5/modules/modules/sunxi_tr.ko:root/sunxi_tr.ko \
    device/softwinner/bpi-m3-lcd5/modules/modules/disp.ko:root/disp.ko \
    device/softwinner/bpi-m3-lcd5/modules/modules/sw-device.ko:recovery/root/sw-device.ko \
    device/softwinner/bpi-m3-lcd5/modules/modules/gt9xxnew_ts.ko:recovery/root/gt9xxnew_ts.ko \

PRODUCT_COPY_FILES += \
    device/softwinner/bpi-m3-lcd5/configs/camera.cfg:system/etc/camera.cfg \
    device/softwinner/bpi-m3-lcd5/configs/cfg-Gallery2.xml:system/etc/cfg-Gallery2.xml \
    device/softwinner/bpi-m3-lcd5/configs/gsensor.cfg:system/usr/gsensor.cfg \
    device/softwinner/bpi-m3-lcd5/configs/media_profiles.xml:system/etc/media_profiles.xml \
    device/softwinner/bpi-m3-lcd5/configs/sunxi-keyboard.kl:system/usr/keylayout/sunxi-keyboard.kl \
    device/softwinner/bpi-m3-lcd5/configs/sunxi-ir.kl:system/usr/keylayout/sunxi-ir.kl \
    device/softwinner/bpi-m3-lcd5/configs/tp.idc:system/usr/idc/tp.idc

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:system/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.ethernet.xml:system/etc/permissions/android.hardware.ethernet.xml \
	frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.software.verified_boot.xml:system/etc/permissions/android.software.verified_boot.xml



PRODUCT_COPY_FILES += \
    device/softwinner/bpi-m3-lcd5/bluetooth/bt_vendor.conf:system/etc/bluetooth/bt_vendor.conf

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml   \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.software.managed_users.xml:system/etc/permissions/android.software.managed_users.xml

PRODUCT_COPY_FILES += \
	device/softwinner/bpi-m3-lcd5/media/bootanimation.zip:system/media/bootanimation.zip \

#camera config for camera detector
#PRODUCT_COPY_FILES += \
#        device/softwinner/bpi-m3-lcd5/hawkview/sensor_list_cfg.ini:system/etc/hawkview/sensor_list_cfg.ini

# Radio Packages and Configuration Flie
$(call inherit-product, device/softwinner/common/rild/radio_common.mk)
#$(call inherit-product, device/softwinner/common/ril_modem/huawei/mu509/huawei_mu509.mk)
#$(call inherit-product, device/softwinner/common/ril_modem/Oviphone/em55/oviphone_em55.mk)

# Justin Porting 20171214 Start  
# camera config for isp
PRODUCT_COPY_FILES += \
    device/softwinner/octopus-common/hawkview/8M/ov8865_4lane/isp_3a_param.ini:system/etc/hawkview/ov8865_4lane/isp_3a_param.ini \
   device/softwinner/octopus-common/hawkview/8M/ov8865_4lane/isp_iso_param.ini:system/etc/hawkview/ov8865_4lane/isp_iso_param.ini \
    device/softwinner/octopus-common/hawkview/8M/ov8865_4lane/isp_test_param.ini:system/etc/hawkview/ov8865_4lane/isp_test_param.ini \
   device/softwinner/octopus-common/hawkview/8M/ov8865_4lane/isp_tuning_param.ini:system/etc/hawkview/ov8865_4lane/isp_tuning_param.ini \
   device/softwinner/octopus-common/hawkview/8M/ov8865_4lane/bin/gamma_tbl.bin:system/etc/hawkview/ov8865_4lane/bin/gamma_tbl.bin \
   device/softwinner/octopus-common/hawkview/8M/ov8865_4lane/bin/hdr_tbl.bin:system/etc/hawkview/ov8865_4lane/bin/hdr_tbl.bin \
    device/softwinner/octopus-common/hawkview/8M/ov8865_4lane/bin/lsc_tbl.bin:system/etc/hawkview/ov8865_4lane/bin/lsc_tbl.bin

# Justin Porting 20171214 End
# gps
$(call inherit-product, device/softwinner/bpi-m3-lcd5/gps/gps.mk)

PRODUCT_PROPERTY_OVERRIDES += \
   ro.frp.pst=/dev/block/by-name/frp

# usb
#Justin 20171206 Porting Start
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp,adb\
    ro.adb.secure=0 \
    ro.sys.mutedrm=true \
    rw.logger=0
#Justin 20171206 Porting End

PRODUCT_PROPERTY_OVERRIDES += \
    ro.product.firmware=v6.0rc4

PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.heapsize=512m \
    dalvik.vm.heapstartsize=8m \
    dalvik.vm.heapgrowthlimit=192m \
    dalvik.vm.heaptargetutilization=0.75 \
    dalvik.vm.heapminfree=2m \
    dalvik.vm.heapmaxfree=8m \
    ro.zygote.disable_gl_preload=true

#Justin 20171206 Porting Start
PRODUCT_PROPERTY_OVERRIDES += \
    ro.property.tabletUI=false \
    ro.sf.lcd_density=180 \
#Justin 20171206 Porting End

# function
# Justin Porting 20171206 Start 
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sys.bootfast=true \
    ro.dmic.used=false
# Justin Porting 20171206 End

# Justin Porting 20171206 Start 
PRODUCT_PROPERTY_OVERRIDES += \
   	persist.sys.timezone=Asia/Taipei \
	persist.sys.language=EN \
	persist.sys.country=US
# Justin Porting 20171206 End
PRODUCT_AAPT_CONFIG := large xlarge hdpi xhdpi
PRODUCT_AAPT_PERF_CONFIG := xhdpi
PRODUCT_CHARACTERISTICS := tablet

# stoarge
PRODUCT_PROPERTY_OVERRIDES += \
    persist.fw.force_adoptable=true

$(call inherit-product-if-exists, vendor/google/products/gms_base.mk)

PRODUCT_BRAND := Allwinner
PRODUCT_NAME := bpi_m3_lcd5
PRODUCT_DEVICE := bpi-m3-lcd5
PRODUCT_MODEL :=  BPI-M3
PRODUCT_MANUFACTURER := SINOVOIP
