[BuildOptions.common]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES -D HAS_MEMCPY_INTRINSICS

[PcdsFixedAtBuild.common]
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemFamily|"MacBook Pro TouchBar"
  #will be changed later on, default values
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferWidth|3456
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferHeight|2234
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferPixelBpp|30

[PcdsDynamicDefault.common]
  #borrowed from SurfaceDuoPkg
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|3456
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|2234
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|3456
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|2234
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|300
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|50
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|300 
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|50

[Components.common]
  MacBookProTouchBarFamilyPkg/AcpiTables/DeviceFamilyAcpiTables.inf