[BuildOptions.common]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES -D HAS_MEMCPY_INTRINSICS

[PcdsFixedAtBuild.common]
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemFamily|"MacBook Pro (with Touch Bar)"
  #will be changed later on, default values
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferWidth|2560
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferHeight|1600
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferPixelBpp|30

[PcdsDynamicDefault.common]
  #borrowed from SurfaceDuoPkg
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|2560
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1600
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|2560
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1600
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|300
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|50
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|300 
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|50

[Components.common]
  MacBookProTouchBarFamilyPkg/AcpiTables/DeviceFamilyAcpiTables.inf