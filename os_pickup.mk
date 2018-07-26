ifeq ($(TARGET_BOARD_PLATFORM),sm8150)
LOCAL_PATH := $(call my-dir)
subdir_makefiles=$(call first-makefiles-under,$(LOCAL_PATH))
$(foreach mk,$(subdir_makefiles),$(info including $(mk) ...)$(eval include $(mk)))
endif
