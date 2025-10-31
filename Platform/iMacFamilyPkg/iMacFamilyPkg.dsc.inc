[BuildOptions.common]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES -D HAS_MEMCPY_INTRINSICS

[PcdsFixedAtBuild.common]
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemFamily|"iMac"
  #will be changed later on, default values
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferWidth|4480
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferHeight|2520
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferPixelBpp|30

[PcdsDynamicDefault.common]
  #borrowed from SurfaceDuoPkg
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|4480
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|2520
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|4480
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|2520
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|300
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|50
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|300 
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|50

[Components.common]
  iMacFamilyPkg/AcpiTables/DeviceFamilyAcpiTables.inf