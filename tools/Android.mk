LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE        := aromafm.zip
LOCAL_MODULE_TAGS   := eng

AROMAFM_LOCALPATH := $(LOCAL_PATH)/../.
AROMA_OUT_PATH := $(TARGET_RECOVERY_ROOT_OUT)/../../aromafm_out
AROMA_DEVICE_NAME := $(shell echo $(TARGET_PRODUCT) | cut -d _ -f 2)

$(info ==========================================================================)
$(info )
$(info Making Aroma Installer Zip)
OUTPUT_SH := $(shell $(AROMAFM_LOCALPATH)/tools/android_building.sh $(AROMAFM_LOCALPATH) $(AROMA_OUT_PATH) $(AROMA_DEVICE_NAME))

ifeq ($(OUTPUT_SH),0)
    $(info Please Compile AROMA Installer First, by running: mmm bootable/recovery/aromafm)
else
    $(info Aroma Zip copied to $(AROMA_OUT_PATH)/aromafm_$(AROMA_DEVICE_NAME).zip)
endif

$(info )
$(info ==========================================================================)
