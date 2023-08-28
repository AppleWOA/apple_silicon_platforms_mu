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
  # Setting timer frequency to 0 means "read from CNTFRQ_EL0"
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|0
  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x8FF000000 #fair warning this location is *totally random*
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|19 # placeholder 
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|17 # phys timer IRQ number (this is arbitrary because per-cpu fiq)
  gArmTokenSpaceGuid.PcdArmArchTimerHypIntrNum|20 #this disables the HV timer
  gArmTokenSpaceGuid.PcdArmArchTimerVirtIntrNum|18 #virt timer
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleSartVersion|2
!if $(USES_MAC_CPU) == TRUE
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuIdentifier|"T8103"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuModel|"Apple M1"
!else
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuIdentifier|"T8101"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuModel|"Apple A14-compatible CPU"
!endif
  #Serial port base addr
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleUartBase|0x235200000
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleSocIdentifier|0x8103
  gAppleSiliconPkgTokenSpaceGuid.PcdInitializeRamdisk|TRUE
  gAppleSiliconPkgTokenSpaceGuid.PcdSecPhaseStackBase|0x900000000

#AIC/GIC configuration settings
!if $(AIC_BUILD) == FALSE
  #
  # if GIC is used, it's expected to be virtualized by trusted firmware at the following addresses.
  # as Tonga/T8103 and Sicily/T8101 has no onboard GIC. (but do contain the MSRs necessary for accelerated virt)
  #
  gArmTokenSpaceGuid.PcdGicDistributorBase|0xf00000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0xf10000000
!else
  #this is MMIO space where the AIC lives
  gAppleSiliconPkgTokenSpaceGuid.PcdAicInterruptControllerBase|0x23b100000

!endif
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00008103

!if $(SECURE_BOOT_ENABLE) == TRUE
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|600
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|400
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|1500
!else
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|300
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|150
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|1000
!endif
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesData|800
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderCode|10
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderData|0
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|40
  
  #PCIe settings
  gEfiMdeModulePkgTokenSpaceGuid.PcdSrIovSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdAriSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdMrIovSupport|FALSE
  #placeholder settings
  gArmTokenSpaceGuid.PcdPciBusMin|0x0
  gArmTokenSpaceGuid.PcdPciBusMax|0xFF

[LibraryClasses.common]
  MemoryInitPeiLib|T810XFamilyPkg/Library/MemoryInitPeiLib/MemoryInitPeiLib.inf
  MsPlatformDevicesLib  |T810XFamilyPkg/Library/MsPlatformDevicesLib/MsPlatformDevicesLib.inf

[Components.common]

  T810XFamilyPkg/AcpiTables/CPUAcpiTables.inf