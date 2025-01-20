ifeq ($(ENABLE_HYP),true)
$(call soong_config_set,qtidisplay,enable_hyp,true)
endif

ifeq ($(TARGET_EXCLUDES_DISPLAY_PP),true)
$(call soong_config_set,qtidisplay,excludes_display_pp,true)
endif

ifeq ($(TARGET_EXCLUDES_MULTI_DISPLAY),true)
$(call soong_config_set,qtidisplay,excludes_multi_display,true)
endif

ifeq ($(TARGET_HAS_WIDE_COLOR_DISPLAY),true)
$(call soong_config_set,qtidisplay,has_wide_display,true)
endif

ifeq ($(TARGET_IS_HEADLESS), true)
$(call soong_config_set,qtidisplay,is_headless,true)
endif

ifeq ($(TARGET_USES_DRM_PP),true)
$(call soong_config_set,qtidisplay,uses_drm_pp,true)
endif

ifeq ($(TARGET_USES_GRALLOC4),true)
$(call soong_config_set,qtidisplay,uses_gralloc4,true)
endif

ifeq ($(TARGET_USES_YCRCB_CAMERA_ENCODE),true)
$(call soong_config_set,qtidisplay,uses_ycrcb_camera_encode,true)
endif
