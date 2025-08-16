[PcdsFixedAtBuild.common]
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosSystemFamily|"MacBook Air"
  #will be changed later on, default values
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferWidth|2560
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferHeight|1600
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferPixelBpp|30


[PcdsPatchableInModule.common]



[PcdsDynamicDefault.common]
  #screen resolution settings
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|2560
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1600
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|2560
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1600
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|150
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|150
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|150
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|150

[Components.common]
  MacBookAirFamilyPkg/AcpiTables/DeviceFamilyAcpiTables.inf