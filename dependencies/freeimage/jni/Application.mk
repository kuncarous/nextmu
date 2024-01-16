ADDITIONAL_FLAGS := -O3 -fopenmp -DANDROID -static-openmp -fopenmp
APP_CFLAGS += $(ADDITIONAL_FLAGS)
APP_CONLYFLAGS += -std=c11
APP_CPPFLAGS += $(ADDITIONAL_FLAGS) -std=c++11 -fexceptions -frtti
APP_PLATFORM := android-21
APP_STL   := c++_static
APP_ABI   := armeabi-v7a arm64-v8a x86_64 x86
APP_OPTIM := release
APP_SHORT_COMMANDS := true
