LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := MT2D

SDL_PATH := ../SDL
SDL_IMAGE_PATH := ../SDL2_image
SDL_MIXER_PATH := ../SDL2_mixer



LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \ $(LOCAL_PATH)/$(SDL_MIXER_PATH) \ $(LOCAL_PATH)/$(SDL_IMAGE_PATH)

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/MT2D/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/Audio/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/Container/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/Cursor/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/InputBox/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/Joystick/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/MessageBox/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/ObjectCore/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/ObjectCore/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/SDL/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/SDL/Audio/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/SDL/IO/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/SDL/Render/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/Tools/UserInterface/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/_ANDROID/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/_ANDROID/IO/*.c) \
	$(wildcard $(LOCAL_PATH)/MT2D/Tools/Video/*.c))

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_mixer SDL2_image

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)
