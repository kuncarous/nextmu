LOCAL_PATH := $(call my-dir)

include $(LOCAL_PATH)/Makefile.srcs

include $(CLEAR_VARS)
LOCAL_MODULE       := FreeImage
LOCAL_STATIC_LIBRARIES       := FreeImage
LOCAL_C_INCLUDES   := $(LOCAL_PATH)/../../Source \
                        $(LOCAL_PATH)/../../Source/Metadata \
                        $(LOCAL_PATH)/../../Source/FreeImageToolkit \
                        $(LOCAL_PATH)/../../Source/LibJPEG \
						$(LOCAL_PATH)/../../Source/LibJXR \
						$(LOCAL_PATH)/../../Source/LibJXR/common/include \
						$(LOCAL_PATH)/../../Source/LibJXR/jxrgluelib \
						$(LOCAL_PATH)/../../Source/LibJXR/image/sys \
						$(LOCAL_PATH)/../../Source/LibOpenJPEG \
                        $(LOCAL_PATH)/../../Source/LibPNG \
						$(LOCAL_PATH)/../../Source/LibRawLite \
                        $(LOCAL_PATH)/../../Source/LibRawLite/dcraw \
                        $(LOCAL_PATH)/../../Source/LibRawLite/internal \
                        $(LOCAL_PATH)/../../Source/LibRawLite/libraw \
                        $(LOCAL_PATH)/../../Source/LibRawLite/src \
                        $(LOCAL_PATH)/../../Source/LibTIFF4 \
						$(LOCAL_PATH)/../../Source/LibWebP \
						$(LOCAL_PATH)/../../Source/LibWebP/src/webp \
						$(LOCAL_PATH)/../../Source/LibWebP/src/utils \
                        $(LOCAL_PATH)/../../Source/OpenEXR \
                        $(LOCAL_PATH)/../../Source/OpenEXR/Half \
						$(LOCAL_PATH)/../../Source/OpenEXR/IexMath \
                        $(LOCAL_PATH)/../../Source/OpenEXR/Iex \
                        $(LOCAL_PATH)/../../Source/OpenEXR/IlmImf \
                        $(LOCAL_PATH)/../../Source/OpenEXR/IlmThread \
                        $(LOCAL_PATH)/../../Source/OpenEXR/Imath \
						$(LOCAL_PATH)/../../Source/ZLib

ifneq ($(findstring armeabi-v7a, $(TARGET_ARCH_ABI)),)
  # Setting LOCAL_ARM_NEON will enable -mfpu=neon which may cause illegal
  # instructions to be generated for armv7a code. Instead target the neon code
  # specifically.
  NEON           := c.neon
  LOCAL_ARM_NEON := true
else
  NEON := c
endif

LOCAL_NEON_SRC         := $(LOCAL_PATH)/../../Source/LibWebP/./src/dsp/dsp.dec_neon.$(NEON) \
						  $(LOCAL_PATH)/../../Source/LibWebP/./src/dsp/dsp.enc_neon.$(NEON) \
						  $(LOCAL_PATH)/../../Source/LibWebP/./src/dsp/dsp.lossless_neon.$(NEON) \
						  $(LOCAL_PATH)/../../Source/LibWebP/./src/dsp/dsp.upsampling_neon.$(NEON)

$(SRCS) += $(LOCAL_NEON_SRC)
LOCAL_SRC_FILES        := $(SRCS)
#LOCAL_LDLIBS           := -L$(SYSROOT)/usr/lib -llog -landroid
#LOCAL_LDFLAGS          += -static-openmp -fopenmp
LOCAL_ADDITIONAL_FLAGS := -O3 -fPIC -DPNG_ARM_NEON_OPT=0 -D__ANSI__ -DDISABLE_PERF_MEASUREMENT -DHAVE_MALLOC_H -DHAVE_PTHREAD -DWEBP_USE_THREAD -static-openmp -fopenmp
LOCAL_CPPFLAGS         := $(LOCAL_ADDITIONAL_FLAGS) -std=c++11 -frtti -fexceptions
LOCAL_CFLAGS           := $(LOCAL_ADDITIONAL_FLAGS)
LOCAL_ARM_MODE         := arm
LOCAL_STATIC_LIBRARIES := cpufeatures
include $(BUILD_STATIC_LIBRARY)

$(call import-module,android/cpufeatures)
