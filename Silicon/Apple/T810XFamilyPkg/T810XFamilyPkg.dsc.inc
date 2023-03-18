[PcdsPatchableInModule.common]
  #These will get overriden by the FDT settings
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x800000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x200000000 #8GB RAM space by default
  gAppleSiliconPkgTokenSpaceGuid.PcdFdtPointer|0x840000000
  gArmTokenSpaceGuid.PcdFdBaseAddress|0x0
  gArmTokenSpaceGuid.PcdFvBaseAddress|0x0
  #Serial port base addr
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleUartBase|0x235200000
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleSocIdentifier|0x8103
  