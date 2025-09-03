[PcdsPatchableInModule.common]
  #These will get overriden by the FDT settings
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x800000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x200000000 #8GB RAM space by default
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
  gAppleSiliconPkgTokenSpaceGuid.PcdBootArgsPointer|0x840000000
  gAppleSiliconPkgTokenSpaceGuid.PcdAdtPointer|0x840004000
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
  #number of cores (right now we are assuming the lowest common denominator - this will change later)
  gAppleSiliconPkgTokenSpaceGuid.PcdCoreCount|8|UINT32|0x00004406
  #all Rhodes chips have three clusters, "boot", P-core/E-core clusters
  # for multi-die systems, this cluster setup is duplicated on the second die
  gAppleSiliconPkgTokenSpaceGuid.PcdClusterCount|2|UINT32|0x00004407

#AIC/GIC configuration settings
!if $(AIC_BUILD) == FALSE
  #
  # if GIC is used, it's expected to be virtualized by trusted firmware at the following addresses.
  # as Tonga/T8103 and Sicily/T8101 has no onboard GIC. (but do contain the MSRs necessary for accelerated virt)
  #
  gArmTokenSpaceGuid.PcdGicDistributorBase|0xf00000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0xf10000000
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

  #
  # Apple chips don't seem to use "x86-style" I/O accesses, so use placeholder values here, and don't reference them in the host bridge driver.
  #
  gArmTokenSpaceGuid.PcdPciIoBase|0x0
  gArmTokenSpaceGuid.PcdPciIoSize|0x00010000
  #
  # The 32 bit PCIe MMIO is not prefetchable.
  #
  gArmTokenSpaceGuid.PcdPciMmio32Base|0xc0000000
  gArmTokenSpaceGuid.PcdPciMmio32Size|0x40000000
  gEfiMdePkgTokenSpaceGuid.PcdPciMmio32Translation|0x500000000
  gEfiMdePkgTokenSpaceGuid.PcdPciMmio64Translation|0x0
  #
  # 64 bit PCIe MMIO *is* prefetchable.
  #
  gArmTokenSpaceGuid.PcdPciMmio64Base|0x400000000
  gArmTokenSpaceGuid.PcdPciMmio64Size|0x20000000

  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0x690000000
  gAppleSiliconPkgTokenSpaceGuid.PcdPciExpressBarSize|0x1000000
  gAppleSiliconPkgTokenSpaceGuid.PcdPciExpressBarLimit|0x690FFFFFF

  gEmbeddedTokenSpaceGuid.PcdDmaDeviceOffset|0xFFFFFFFB00000000
  #
  # This shouldn't get used.
  #
  gEfiMdePkgTokenSpaceGuid.PcdPciIoTranslation|0x0

[LibraryClasses.common]
  MemoryInitPeiLib|T810XFamilyPkg/Library/MemoryInitPeiLib/MemoryInitPeiLib.inf
  MsPlatformDevicesLib  |T810XFamilyPkg/Library/MsPlatformDevicesLib/MsPlatformDevicesLib.inf

[Components.common]

  T810XFamilyPkg/AcpiTables/CPUAcpiTables.inf