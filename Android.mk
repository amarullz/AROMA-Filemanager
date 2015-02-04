LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

  ## Check for ARM NEON
  AROMA_ARM_NEON      := false
  ifeq ($(ARCH_ARM_HAVE_NEON),true)
    AROMA_ARM_NEON    := true
  endif
  
  ##
  ## Force Compiling Without ARM NEON
  ##   -- Uncomment This Line --
  ##
  # AROMA_ARM_NEON      := false
  #
  
  ##
  ## VERSIONING
  ##
  AROMA_NAME    := AROMA Filemanager
  AROMA_VERSION := 1.92
  AROMA_BUILD   := $(shell date +%y%m%d%H)
  AROMA_CN      := Degung Gamelan

  ## LOCAL PATH COPY
  AROMAFM_LOCALPATH := $(LOCAL_PATH)
  
  ## binary output path
  AROMA_OUT_PATH := $(TARGET_RECOVERY_ROOT_OUT)/../../aromafm_out

  ## ZLIB SOURCE FILES
  LOCAL_SRC_FILES := 	\
    libs/zlib/adler32.c \
    libs/zlib/crc32.c \
    libs/zlib/infback.c \
    libs/zlib/inffast.c \
    libs/zlib/inflate.c \
    libs/zlib/inftrees.c \
    libs/zlib/zutil.c
  ## ZLIB NEON SOURCE
  ifeq ($(AROMA_ARM_NEON),true)
    LOCAL_SRC_FILES += libs/zlib/inflate_fast_copy_neon.s
  endif
  
  ## PNG SOURCE FILES
  LOCAL_SRC_FILES += \
    libs/png/png.c \
    libs/png/pngerror.c \
    libs/png/pnggccrd.c \
    libs/png/pngget.c \
    libs/png/pngmem.c \
    libs/png/pngpread.c \
    libs/png/pngread.c \
    libs/png/pngrio.c \
    libs/png/pngrtran.c \
    libs/png/pngrutil.c \
    libs/png/pngset.c \
    libs/png/pngtrans.c \
    libs/png/pngvcrd.c
  ## PNG NEON SOURCE
  ifeq ($(AROMA_ARM_NEON),true)
    LOCAL_SRC_FILES += libs/png/png_read_filter_row_neon.s
  endif
    
  ## MINUTF8 & MINZIP SOURCE FILES
  LOCAL_SRC_FILES += \
    libs/minutf8/minutf8.c \
    libs/minzip/DirUtil.c \
    libs/minzip/Hash.c \
    libs/minzip/Inlines.c \
    libs/minzip/SysUtil.c \
    libs/minzip/Zip.c
  
  ## FREETYPE SOURCE FILES
  LOCAL_SRC_FILES += \
    libs/freetype/autofit/autofit.c \
    libs/freetype/base/basepic.c \
    libs/freetype/base/ftapi.c \
    libs/freetype/base/ftbase.c \
    libs/freetype/base/ftbbox.c \
    libs/freetype/base/ftbitmap.c \
    libs/freetype/base/ftglyph.c \
    libs/freetype/base/ftinit.c \
    libs/freetype/base/ftpic.c \
    libs/freetype/base/ftstroke.c \
    libs/freetype/base/ftsynth.c \
    libs/freetype/base/ftsystem.c \
    libs/freetype/cff/cff.c \
    libs/freetype/pshinter/pshinter.c \
    libs/freetype/psnames/psnames.c \
    libs/freetype/raster/raster.c \
    libs/freetype/sfnt/sfnt.c \
    libs/freetype/smooth/smooth.c \
    libs/freetype/truetype/truetype.c \
    libs/freetype/base/ftlcdfil.c
  
  ## AROMA CONTROLS SOURCE FILES
  LOCAL_SRC_FILES += \
    src/aroma_openpty.c \
	  src/controls/aroma_controls.c \
    src/controls/aroma_control_button.c \
    src/controls/aroma_control_check.c \
    src/controls/aroma_control_checkbox.c \
    src/controls/aroma_control_console.c \
    src/controls/aroma_control_edit.c \
    src/controls/aroma_control_filebox.c \
    src/controls/aroma_control_ime.c \
    src/controls/aroma_control_ime2.c \
    src/controls/aroma_control_imgbutton.c \
    src/controls/aroma_control_label.c \
    src/controls/aroma_control_menubox.c \
    src/controls/aroma_control_optbox.c \
    src/controls/aroma_control_progress.c \
    src/controls/aroma_control_textbox.c \
    src/controls/aroma_control_threads.c
  
  ## AROMA LIBRARIES SOURCE FILES
  LOCAL_SRC_FILES += \
    src/libs/aroma_array.c \
    src/libs/aroma_freetype.c \
    src/libs/aroma_graph.c \
    src/libs/aroma_input.c \
    src/libs/aroma_languages.c \
    src/libs/aroma_libs.c \
    src/libs/aroma_memory.c \
    src/libs/aroma_png.c \
    src/libs/aroma_zip.c
  
  ## AROMA FILEMANAGER SOURCE FILES
  LOCAL_SRC_FILES += \
    src/main/aroma.c \
    src/main/aroma_ui.c

  ## MODULE SETTINGS
  LOCAL_MODULE                  := aroma_filemanager
  LOCAL_MODULE_TAGS             := eng
  LOCAL_FORCE_STATIC_EXECUTABLE := true
  
  ## INCLUDES & OUTPUT PATH
  LOCAL_C_INCLUDES              := $(AROMAFM_LOCALPATH)/include
  LOCAL_MODULE_PATH             := $(AROMA_OUT_PATH)
  
  ## COMPILER FLAGS
  LOCAL_CFLAGS                  := -O2 
  LOCAL_CFLAGS                  += -DFT2_BUILD_LIBRARY=1 -DDARWIN_NO_CARBON 
  LOCAL_CFLAGS                  += -fdata-sections -ffunction-sections
  LOCAL_CFLAGS                  += -Wl,--gc-sections -fPIC -DPIC
  LOCAL_CFLAGS                  += -D_AROMA_NODEBUG
  
  ## SET VERSION
  LOCAL_CFLAGS += -DAROMA_NAME="\"$(AROMA_NAME)\""
  LOCAL_CFLAGS += -DAROMA_VERSION="\"$(AROMA_VERSION)\""
  LOCAL_CFLAGS += -DAROMA_BUILD="\"$(AROMA_BUILD)\""
  LOCAL_CFLAGS += -DAROMA_BUILD_CN="\"$(AROMA_CN)\""
  
  ifeq ($(AROMA_ARM_NEON),true)
    LOCAL_CFLAGS                  += -mfloat-abi=softfp -mfpu=neon -D__ARM_HAVE_NEON
  endif
  
  ## INCLUDED LIBRARIES
  LOCAL_STATIC_LIBRARIES        := libm libc
  
## Remove Old Build
$(shell rm -rf $(PRODUCT_OUT)/obj/EXECUTABLES/$(LOCAL_MODULE)_intermediates)

## Create zip installer
AROMA_DEVICE_NAME   := $(shell echo $(TARGET_PRODUCT) | cut -d _ -f 2)
AROMA_ZIP_FILE      := $(AROMA_OUT_PATH)/aromafm_$(AROMA_DEVICE_NAME).zip
$(AROMA_ZIP_FILE): aroma_filemanager
	$(info )
	$(info Making Aroma Installer Zip...)
	$(AROMAFM_LOCALPATH)/tools/android_building.sh $(AROMAFM_LOCALPATH) $(AROMA_OUT_PATH) $(AROMA_DEVICE_NAME)
	$(info Install ----> $(AROMA_ZIP_FILE))
	$(info )

ALL_DEFAULT_INSTALLED_MODULES += $(AROMA_ZIP_FILE)

include $(BUILD_EXECUTABLE)
