[PcdsPatchableInModule.common]
  #These will get overriden by the FDT settings
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x800000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x200000000 #8GB RAM space by default
  gAppleSiliconPkgTokenSpaceGuid.PcdFdtPointer|0x840000000
  gArmTokenSpaceGuid.PcdFdBaseAddress|0x0
  gArmTokenSpaceGuid.PcdFvBaseAddress|0x0
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferAddress|0x0
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferSize|0x0

[PcdsFixedAtBuild.common]
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|24000000
  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x8FF000000 #fair warning this location is *totally random*
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|17 #FIQ timer INTID (needed because Apple timers fire FIQs)
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|18 #IRQ timer INTID
!if $(USES_MAC_CPU) == TRUE
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuIdentifier|"T8103"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuModel|"Apple M1"
!else
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuIdentifier|"T810X"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuModel|"Apple A14-compatible CPU"
  #Serial port base addr
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleUartBase|0x235200000
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleSocIdentifier|0x8103
  gAppleSiliconPkgTokenSpaceGuid.PcdInitializeRamdisk|TRUE
  gAppleSiliconPkgTokenSpaceGuid.PcdSecPhaseStackBase|0x900000000

#AIC/GIC configuration settings
!if $(AIC_BUILD) == FALSE
  #On all M1 and M2 models, these addresses are completely unmapped and point to nowhere
  #m1n1 will trap accesses to these addresses, which will emulate the vGIC dist/redist
  gArmTokenSpaceGuid.PcdGicDistributorBase|0xf00000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0xf10000000
!else
  #this is MMIO space where the AIC lives
  gAppleSiliconPkgTokenSpaceGuid.PcdAicInterruptControllerBase|0x23b100000

!endif
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00008103