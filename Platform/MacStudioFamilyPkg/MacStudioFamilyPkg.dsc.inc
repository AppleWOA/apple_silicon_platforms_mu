[BuildOptions.common]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES -D HAS_MEMCPY_INTRINSICS

[PcdsFixedAtBuild.common]
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemFamily|"Mac Studio"

  #will be changed later on, default values
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferWidth|1920
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferHeight|1080
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferPixelBpp|30


[PcdsDynamicDefault.common]

  #borrowed from SurfaceDuoPkg
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1920
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1080
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1920
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1080
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|300
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|50
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|300 
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|50


[Components.common]
  #
  # This is mainly for FADT, but for multi-platform, this helps with porting to other variants of the same platform
  #
  MacStudioFamilyPkg/AcpiTables/DeviceFamilyAcpiTables.inf