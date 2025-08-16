# T602x SoC family DSC include file
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018 - 2019, Bingxing Wang. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#
# SPDX-License-Identifier: BSD-2-Clause OR MIT


[PcdsPatchableInModule.common]
  #These will get overriden by the FDT settings
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x10000000000
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x400000000 #16GB RAM space by default
  gArmTokenSpaceGuid.PcdFdBaseAddress|0x0
  gArmTokenSpaceGuid.PcdFvBaseAddress|0x0
  gAppleSiliconPkgTokenSpaceGuid.PcdFdtPointer|0x10220000000 # TODO: verify this hack still works on T6020
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferAddress|0x0
  gAppleSiliconPkgTokenSpaceGuid.PcdFrameBufferSize|0x0

[PcdsFixedAtBuild.common]
  #ARM architectural fixed build-time settings
  # Setting timer frequency to 0 means "read from CNTFRQ_EL0"
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|0
  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x100FF000000 #fair warning this location is *totally random*
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|19 # placeholder 
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|17 # phys timer IRQ number (this is arbitrary because per-cpu fiq)
  gArmTokenSpaceGuid.PcdArmArchTimerHypIntrNum|20 #this disables the HV timer
  gArmTokenSpaceGuid.PcdArmArchTimerVirtIntrNum|18 #virt timer
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleUartBase|0x39b200000
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleSocIdentifier|0x6020
  gAppleSiliconPkgTokenSpaceGuid.PcdInitializeRamdisk|TRUE
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuIdentifier|"T602x"
  gAppleSiliconPkgTokenSpaceGuid.PcdSmbiosCpuModel|"Apple M2 Pro/Max/Ultra (Rhodes)"
  gAppleSiliconPkgTokenSpaceGuid.PcdSecPhaseStackBase|0x10300000000
  gAppleSiliconPkgTokenSpaceGuid.PcdAppleSartVersion|3
  gAppleSiliconPkgTokenSpaceGuid.PcdXhciPcieDeviceNumber|3
  #number of cores (right now we are assuming the lowest common denominator - this will change later)
  gAppleSiliconPkgTokenSpaceGuid.PcdCoreCount|12|UINT32|0x00004406
  #all Rhodes chips have three clusters, "boot", P-core/E-core clusters
  # for multi-die systems, this cluster setup is duplicated on the second die
  gAppleSiliconPkgTokenSpaceGuid.PcdClusterCount|3|UINT32|0x00004407
#AIC/GIC configuration settings
!if $(AIC_BUILD) == FALSE
  #
  # if GIC is used, it's expected to be virtualized by EL2 at the below addresses 
  # as Rhodes/T602x has no onboard GIC. (but does contain the MSRs to do accelerated virt)
  #
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x5000000000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x5100000000
!else
  #this is MMIO space where the AIC lives
  gAppleSiliconPkgTokenSpaceGuid.PcdAicInterruptControllerBase|0x28e100000

!endif
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00006020

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
  #
  # Host Bridge Library settings.
  # 
  gArmTokenSpaceGuid.PcdPciBusMin|0x0
  gArmTokenSpaceGuid.PcdPciBusMax|0x4
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
  gArmTokenSpaceGuid.PcdPciMmio64Base|0x5A0000000
  gArmTokenSpaceGuid.PcdPciMmio64Size|0x20000000

  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0x580000000
  #
  # This shouldn't get used.
  #
  gEfiMdePkgTokenSpaceGuid.PcdPciIoTranslation|0x0


[LibraryClasses.common]
  MemoryInitPeiLib|T602XFamilyPkg/Library/MemoryInitPeiLib/MemoryInitPeiLib.inf
  MsPlatformDevicesLib  |T602XFamilyPkg/Library/MsPlatformDevicesLib/MsPlatformDevicesLib.inf

[Components.common]

  T602XFamilyPkg/AcpiTables/CPUAcpiTables.inf
