ifeq ($(TARGET_USES_YCRCB_CAMERA_ENCODE),true)
$(call soong_config_set,gralloc,uses_ycrcb_camera_encode,true)
endif

ifeq ($(TARGET_USES_DRM_PP),true)
$(call soong_config_set,sdmcore,uses_drm_pp,true)
endif
