ifeq ($(TARGET_USES_YCRCB_CAMERA_ENCODE),true)
$(call soong_config_set,gralloc,uses_ycrcb_camera_encode,true)
endif

ifeq ($(TARGET_USES_DRM_PP),true)
$(call soong_config_set,sdmcore,uses_drm_pp,true)
endif

ifeq ($(TARGET_EXCLUDES_DISPLAY_PP),true)
$(call soong_config_set,hwc2,excludes_display_pp,true)
endif

ifeq ($(TARGET_EXCLUDES_MULTI_DISPLAY),true)
$(call soong_config_set,hwc2,excludes_multi_display,true)
endif

ifeq ($(TARGET_HAS_WIDE_COLOR_DISPLAY),true)
$(call soong_config_set,hwc2,has_wide_display,true)
endif
