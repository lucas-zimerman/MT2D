LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := MT2D

SDL_PATH := ../SDL2
SDL_MIXER_PATH := ../SDL2_mixer



LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \ $(LOCAL_PATH)/$(SDL_MIXER_PATH)

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/MT2D/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/Audio/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/Cursor/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/InputBox/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/Joystick/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/MessageBox/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/ObjectCore/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/SDL/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/SDL/Audio/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/SDL/IO/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/SDL/Render/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/Tools/UserInterface/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/_ANDROID/*.cpp) \
	$(wildcard $(LOCAL_PATH)/MT2D/Tools/Video/*.cpp))

LOCAL_LDLIBS :=
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := SDL2 SDL2_mixer SDL2_image


LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)
